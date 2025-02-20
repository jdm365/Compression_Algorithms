const std = @import("std");
const builtin = @import("builtin");

const State = struct {
    value: u32,
    bits_left: u32,
};

const BitStream = struct {
    buffer: []u8,
    byte_pos: usize,
    bit_idx: u3,
    
    fn init(buffer: []u8) BitStream {
        return .{
            .buffer = buffer,
            .byte_pos = 0,
            .bit_idx = 0,
        };
    }
    
    fn addBits(self: *BitStream, bits: u32, nb_bits: u5) void {
        var current_byte = switch (self.bit_idx) {
            0 => 0,
            else => self.buffer[self.position],
        };
        
        current_byte |= @as(u8, @intCast(bits << self.bit_position));
        self.buffer[self.position] = current_byte;
        
        const bits_written = @as(u32, @intCast(8 - self.bit_position));
        if (nb_bits < bits_written) {
            self.bit_position += nb_bits;
            return;
        }
        
        var remaining_bits = nb_bits - bits_written;
        var remaining_value = bits >> bits_written;
        
        self.position += 1;
        while (remaining_bits >= 8) {
            self.buffer[self.position] = @as(u8, @intCast(remaining_value));
            remaining_value >>= 8;
            remaining_bits -= 8;
            self.position += 1;
        }
        
        if (remaining_bits > 0) {
            self.buffer[self.position] = @as(u8, @intCast(remaining_value));
            self.bit_position = remaining_bits;
        } else {
            self.bit_position = 0;
        }
    }
};

fn encodeSymbol(state: *State, symbol: u8, stream: *BitStream) void {
    // Get next state from transition table using both state and symbol
    var entry: ?TT_Entry = null;
    for (TT) |e| {
        if (e.symbol == symbol and e.next_state == state.value) {
            entry = e;
            break;
        }
    }
    if (entry == null) @panic("Invalid state transition");
    const next_state = entry.?.next_state;
    
    // Calculate number of bits to output
    const nb_bits = @as(u5, @intCast(TABLE_LOG)) - @as(u5, @intCast(
        @clz(next_state + 1) - (32 - TABLE_LOG)
    ));
    
    // Output nb_bits from state
    if (nb_bits > 0) {
        stream.addBits(state.value, nb_bits);
    }
    
    // Update state
    state.value = next_state;
}

fn compress(input: []const u8, output: []u8) usize {
    var stream = BitStream.init(output);
    var state = State{ .value = 0, .bits_left = 0 };
    
    // Store last byte uncompressed to help with decompression
    const last_byte = input[input.len - 1];
    stream.addBits(last_byte, 8);
    
    // Encode in reverse
    var i: usize = input.len - 1;
    while (i > 0) : (i -= 1) {
        encodeSymbol(&state, input[i], &stream);
    }
    
    // Flush remaining state bits
    stream.addBits(state.value, TABLE_LOG);
    
    // return stream.position + if (stream.bit_position > 0) 1 else 0;
    return stream.position + @intFromBool(stream.bit_position > 0);
}

var SCRATCH_BUFFER: [16_384]u8 = undefined;
var FREQ_TABLE: [256]usize = undefined;

const TT_Entry = struct {
    symbol: u8,
    next_state: u16,
};

// Transition Table
const TT_SIZE = 256;
var TT: [TT_SIZE]TT_Entry = undefined;

const endianness = builtin.cpu.arch.endian();
const big_endian = std.builtin.Endian.big;

const TABLE_LOG = 8;

fn buildFrequencyTable(buffer: []const u8) void {
    // When optimizing later, could do map reduce with multiple tables
    // to try to get IPC benefits.
    @memset(&FREQ_TABLE, 0);

    for (buffer) |b| {
        FREQ_TABLE[b] += 1;
    }
}

fn printFrequencyTable() void {
    for (0.., FREQ_TABLE) |idx, freq| {
        if (freq != 0) {
            std.debug.print("{d}: {c} | {d}\n", .{idx, @as(u8, @intCast(idx)), freq});
        }
    }
}

fn normalizeFrequencyTable() void {
    var total: usize = 0;
    var num_symbols: usize = 0;
    for (FREQ_TABLE) |freq| {
        if (freq > 0) {
            total += freq;
            num_symbols += 1;
        }
    }
    
    if (total == 0 or num_symbols == 0) return;

    const distributable_size = TT_SIZE - num_symbols;
    const scale = @as(f64, @floatFromInt(distributable_size)) / 
                  @as(f64, @floatFromInt(total));
    
    var remaining_total: usize = TT_SIZE;
    for (&FREQ_TABLE) |*freq| {
        if (freq.* == 0) continue;
        
        var new_freq = @as(usize, @intFromFloat(
                @as(f64, @floatFromInt(freq.*)) * scale
                ));
        if (new_freq == 0) new_freq = 1;
        
        freq.* = new_freq;
        remaining_total -= new_freq;
    }

    while (remaining_total > 0) {
        var max_freq: usize = 0;
        var max_idx:  usize = 0;
        for (&FREQ_TABLE, 0..) |freq, idx| {
            if (freq > max_freq) {
                max_freq = freq;
                max_idx = idx;
            }
        }
        if (max_freq == 0) break;
        
        FREQ_TABLE[max_idx] += 1;
        remaining_total -= 1;
    }
}

fn buildTransitionTable() void {
    var symbol_bits: [256]u8 = undefined;
    for (FREQ_TABLE, 0..) |freq, sym| {
        if (freq == 0) continue;
        const bits = std.math.log2(TT_SIZE / freq);
        symbol_bits[sym] = @intCast(bits);
    }

    var offsets: [256]u16   = undefined;
    var current_offset: u16 = 0;
    
    for (FREQ_TABLE, 0..) |freq, sym| {
        if (freq == 0) continue;
        offsets[sym] = current_offset;
        current_offset += @as(u16, @intCast(freq));
    }

    for (FREQ_TABLE, 0..) |freq, sym| {
        if (freq == 0) continue;
        const bits = symbol_bits[sym];
        const offset = offsets[sym];
        
        var pos: usize = 0;
        const step = TT_SIZE / freq;
        
        while (pos < TT_SIZE) {
            const state = @as(u16, @intCast(pos));
            const next_state = (state >> @as(u4, @intCast(bits))) + offset;
            
            TT[pos] = .{
                .symbol = @as(u8, @intCast(sym)),
                .next_state = next_state,
            };
            
            pos += step;
        }
    }
}


pub fn main() !void {
    var arena = std.heap.ArenaAllocator.init(std.heap.page_allocator);
    defer arena.deinit();

    var _decompress = false;
    var filename: []const u8 = undefined;

    var args = try std.process.argsWithAllocator(arena.allocator());
    defer args.deinit();
    var idx: usize = 0;
    while (args.next()) |arg| {
        if (idx == 0) {
            // Skip binary name.
        } else if (idx == 1) {
            std.debug.print("ARG: {s}\n", .{arg});
            if (std.mem.eql(u8, arg, "-d")) {
                _decompress = true;
            } else {
                filename = arg;
            }
        } else if (idx == 2) {
            if (!_decompress) {
                @panic("Too many arguments");
            }
            filename = arg;
        } else {
            @panic("Too many arguments.");
        }
        idx += 1;
    }
    if (idx == 1) {
        // filename = "../../data/enwik8";
        filename = "../../data/declaration_of_independence.txt";
    }

    // For now just read whole file into buffer.
    const input_file = try std.fs.cwd().openFile(filename, .{});
    defer input_file.close();

    const file_size = try input_file.getEndPos();
    const buffer = try arena.allocator().alloc(u8, file_size);
    const output_buffer = try arena.allocator().alloc(u8, file_size);

    _ = try input_file.readAll(buffer);
    std.debug.print("Buffer: {s}\n", .{buffer[0..64]});

    // buildFrequencyTable(buffer);
    // normalizeFrequencyTable();
    // buildTransitionTable();
    // std.debug.print("TT: {any}\n", .{TT});

    const compressed_size = compress(buffer, output_buffer);
    std.debug.print("Compressed size: {d}\n", .{compressed_size});

    // if (_decompress) {
        // try stream.decompress(arena.allocator());
    // } else {
        // try stream.compress(arena.allocator());
    // }
}
