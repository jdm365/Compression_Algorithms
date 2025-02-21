const std = @import("std");
const builtin = @import("builtin");

const State = struct {
    value: u32,
    bits_left: u32,
};

const BitStream = struct {
    input_buffer: []const u8,
    output_buffer: []u64,
    i_buffer_idx: usize,
    i_bit_idx: usize,
    o_buffer_idx: usize,
    o_bit_idx: usize,
    
    fn init(input_buffer: []const u8, output_buffer: []u64) BitStream {
        return BitStream{
            .input_buffer = input_buffer,
            .output_buffer = output_buffer,
            .i_buffer_idx = 0,
            .i_bit_idx = 0,
            .o_buffer_idx = 0,
            .o_bit_idx = 0,
        };
    }
    
    inline fn addBits(self: *BitStream, nb_bits: u8) void {
        const overflow = nb_bits > 63 - self.bit_idx;

        self.output_buffer[self.buffer_idx] |= (bits << @as(u6, @truncate(self.bit_idx)));

        if (overflow) {
            self.buffer_idx += 1;
            self.output_buffer[self.buffer_idx] |= (bits >> @as(u6, @truncate(63 - self.bit_idx)));
        }

        self.bit_idx = (self.bit_idx + nb_bits) % 64;
    }
};

inline fn encodeSymbol(state: *State, stream: *BitStream) void {
    // Get next state from transition table using both state and symbol
    const entry = TT[state.value];
    stream.addBits(state.value, entry.num_bits);
    
    state.value = entry.next_state + ;
}

fn compress(input: []const u8, output: []u64) usize {
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
    
    return 8 * stream.buffer_idx + @divFloor(stream.bit_idx, 8);
}

var SCRATCH_BUFFER: [16_384]u8 = undefined;
var FREQ_TABLE: [256]usize = undefined;

const TT_Entry = packed struct (u32){
    symbol: u8,
    next_state: u16,
    num_bits: u8,
};

// Transition Table
const TABLE_LOG: usize = 8;
const TT_SIZE = 1 << TABLE_LOG;
var TT: [TT_SIZE]TT_Entry = undefined;

const endianness = builtin.cpu.arch.endian();
const big_endian = std.builtin.Endian.big;


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
            const state = @as(u16, @truncate(pos));
            const next_state = (state >> @as(u4, @truncate(bits))) + offset;
            
            TT[pos] = .{
                .symbol = @as(u8, @truncate(sym)),
                .next_state = next_state,
                .num_bits = bits,
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
    const output_buffer = try arena.allocator().alloc(
        u64, 
        try std.math.divCeil(usize, file_size, 8),
        );

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
