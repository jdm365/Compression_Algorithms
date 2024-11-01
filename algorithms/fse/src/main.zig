const std = @import("std");
const builtin = @import("builtin");


const BUFFER_SIZE: usize = (1 << 22);
var SCRATCH_BUFFER: [4096]u8 = undefined;
const endianness = builtin.cpu.arch.endian();


const HuffmanNode = struct {
    value: u8,
    freq: u32,
    left:  ?*HuffmanNode,
    right: ?*HuffmanNode,
};

fn lessThan(context: void, a: *HuffmanNode, b: *HuffmanNode) std.math.Order {
    _ = context;
    return std.math.order(a.freq, b.freq);
}

inline fn readValFromFile(
    comptime T: type,
    file: *std.fs.File,
) !T {
    var _val: [@sizeOf(T)]u8 = undefined;
    _ = try file.read(std.mem.asBytes(&_val));
    return std.mem.readInt(T, &_val, endianness);
}


pub fn buildHuffmanTree(
    allocator: std.mem.Allocator,
    buffer: []u8,
    root: *?*HuffmanNode,
) !void {
    // To start, do this on a chunk by chunk level.
    var freqs: [256]usize = undefined;
    @memset(freqs[0..256], 0);

    var idx: usize = 0;
    var idx_0: usize = 0;
    var idx_1: usize = 0;
    var idx_2: usize = 0;
    var idx_3: usize = 0;

    while (idx < BUFFER_SIZE) : (idx += 4) {
        idx_0 = @intCast(buffer[idx]);
        idx_1 = @intCast(buffer[idx+1]);
        idx_2 = @intCast(buffer[idx+2]);
        idx_3 = @intCast(buffer[idx+3]);

        freqs[idx_0] += 1;
        freqs[idx_1] += 1;
        freqs[idx_2] += 1;
        freqs[idx_3] += 1;
    }

    var pq = std.PriorityQueue(*HuffmanNode, void, lessThan).init(allocator, {});
    defer pq.deinit();

    for (0.., freqs[0..256]) |i, freq| {
        if (freq > 0) {
            const new_node: *HuffmanNode = try allocator.create(HuffmanNode);
            new_node.* = HuffmanNode{
                .value = @truncate(i),
                .freq = @truncate(freq),
                .left = null,
                .right = null,
            };
            try pq.add(new_node);
        }
    }

    while (pq.count() > 1) {
        const left  = pq.remove();
        const right = pq.remove();

        const parent: *HuffmanNode = try allocator.create(HuffmanNode);
        parent.* = HuffmanNode{
            .value = 0,
            .freq = left.freq + right.freq,
            .left = left,
            .right = right,
        };

        try pq.add(parent);
    }

    root.* = pq.remove();
}

pub fn serializeHuffmanTree(
    root: ?*HuffmanNode,
    stream: *BitStream,
) !void {
    if (root) |_root| {
        _ = try stream.output_file.write(
            std.mem.asBytes(&_root.value),
            );
        _ = try stream.output_file.write(
            std.mem.asBytes(&_root.freq),
            );

        try serializeHuffmanTree(_root.left, stream);
        try serializeHuffmanTree(_root.right, stream);
    } else {
        _ = try stream.output_file.write(
            std.mem.asBytes(&@as(i32, @intCast(-1))),
            );
        return;
    }
}

pub fn deserializeHuffmanTree(
    root: *?*HuffmanNode,
    stream: *BitStream,
    allocator: std.mem.Allocator,
) !void {
    const val = try readValFromFile(i32, &stream.input_file);
    if (val == -1) return;
    try stream.input_file.seekBy(-4);

    if (root.*) |_root| {
        _ = try stream.input_file.read(
            std.mem.asBytes(&_root.value),
            );
        _ = try stream.input_file.read(
            std.mem.asBytes(&_root.freq),
            );
        const new_node = try allocator.create(HuffmanNode);
        new_node.* = HuffmanNode{
            .value = try readValFromFile(u8, &stream.input_file),
            .freq = try readValFromFile(u32, &stream.input_file),
            .left = undefined,
            .right = undefined,
        };
        root.* = new_node;

        try deserializeHuffmanTree(&new_node.left, stream, allocator);
        try deserializeHuffmanTree(&new_node.right, stream, allocator);
    } else {
        @panic("Null node without flag from data.");
    }
}

fn gatherCodes(
    _root: ?*HuffmanNode,
    codes: *[256]u32,
    code_lengths: *[256]u8,
    current_code: *u32,
    current_code_length: u8,
) void {
    if (_root) |root| {
        const left_null  = (root.left == null);
        const right_null = (root.right == null);

        if (left_null and right_null) {
            const value = @as(usize, @intCast(root.value));

            codes[value] = current_code.*;
            code_lengths[value] = current_code_length;
            return;
        }

        current_code.* <<= 1;

        if (!left_null) {
            gatherCodes(
                root.left,
                codes,
                code_lengths,
                current_code,
                current_code_length + 1,
            );
        }
        if (!right_null) {
            gatherCodes(
                root.right,
                codes,
                code_lengths,
                current_code,
                current_code_length + 1,
            );
        }

    } else {
        @panic("Error while gathering codes. Null nodes passed to function.");
    }
}

fn huffmanCompress(
    root: *?*HuffmanNode,
    stream: *BitStream,
    allocator: std.mem.Allocator,
) !void {
    try buildHuffmanTree(allocator, stream.input_buffer, root);
    try serializeHuffmanTree(root.*, stream);

    // Build code table.
    var codes: [256]u32 = undefined;
    var code_lengths: [256]u8 = undefined;
    @memset(&codes, 0);
    @memset(&code_lengths, 0);

    var curr_code: u32 = 0;
    gatherCodes(
        root.*,
        &codes,
        &code_lengths,
        &curr_code,
        0,
    );

    // TODO: Try doing 4 elements at a time. Maybe go from u32 -> u64 or larger.
    for (stream.input_buffer[0..stream.input_buffer_size]) |byte| {
        const ubyte: usize = @intCast(byte);
        const nbits = code_lengths[ubyte];

        const bit_idx = stream.compression_buffer_bit_idx;
        const shift_len: u5 = @intCast(32 - (nbits - bit_idx));
        const code  = codes[ubyte] << shift_len;

        const buf_idx = stream.compression_buffer_idx;

        const ptr: *u32 = @ptrCast(@constCast(&stream.compression_buffer[buf_idx..buf_idx+4]));
        ptr.* |= code;

        stream.compression_buffer_bit_idx += nbits;
        stream.compression_buffer_idx += (stream.compression_buffer_bit_idx / 8);
        stream.compression_buffer_bit_idx %= 8;
    }

    try stream.flushChunk();
}

const BitStream = struct {
    input_file: std.fs.File,
    output_file: std.fs.File,
    // input_buffer: [BUFFER_SIZE]u8,
    // compression_buffer: [BUFFER_SIZE]u8,
    input_buffer: []u8,
    input_buffer_size: usize,
    compression_buffer: []u8,
    input_file_size: usize,
    compressed_file_size: usize,
    input_buffer_idx: usize,
    compression_buffer_idx: usize,
    compression_buffer_bit_idx: usize,

    pub fn init(
        input_filename: []const u8,
        allocator: std.mem.Allocator,
    ) !BitStream {
        @memcpy(SCRATCH_BUFFER[0..input_filename.len], input_filename);
        @memcpy(SCRATCH_BUFFER[input_filename.len..input_filename.len+4], ".fse");

        const input_file  = try std.fs.cwd().openFile(input_filename, .{});
        const output_file = try std.fs.cwd().createFile(
            SCRATCH_BUFFER[0..input_filename.len+4], 
            .{ .read = true },
            );
        const input_file_size = try input_file.getEndPos();

        const input_buffer = try allocator.alloc(u8, BUFFER_SIZE);
        var compression_buffer = try allocator.alloc(u8, BUFFER_SIZE);
        @memset(compression_buffer[0..], 0);

        return BitStream{
            .input_file = input_file,
            .output_file = output_file,
            // .input_buffer = undefined,
            // .compression_buffer = undefined,
            .input_buffer = input_buffer,
            .input_buffer_size = 0,
            .compression_buffer = compression_buffer,
            .input_file_size = input_file_size,
            .compressed_file_size = 0,
            .input_buffer_idx = 0,
            .compression_buffer_idx = 0,
            .compression_buffer_bit_idx = 0,
        };
    }

    pub fn deinit(self: *BitStream) void {
        self.input_file.close();
        self.output_file.close();
    }

    pub fn flushChunk(self: *BitStream) !void {
        _ = try self.output_file.write(
            self.compression_buffer[0..self.compression_buffer_idx]
            );
        self.compressed_file_size += self.compression_buffer_idx;
        @memset(self.compression_buffer[0..self.compression_buffer_idx], 0);

        self.compression_buffer_idx     = 0;
        self.compression_buffer_bit_idx = 0;
    }

    pub fn readChunk(self: *BitStream) !bool {
        const bytes_read = try self.input_file.read(
            self.input_buffer[0..BUFFER_SIZE]
            );
        self.input_buffer_size = bytes_read;

        return (bytes_read < BUFFER_SIZE);
    }

    pub fn compress(self: *BitStream, allocator: std.mem.Allocator) !void {
        const start = std.time.microTimestamp();

        var done = false;
        while (!done) {
            done = try self.readChunk();
            var root: ?*HuffmanNode = null;
            try huffmanCompress(&root, self, allocator);
        }

        const time_taken_us = std.time.microTimestamp() - start;
        const mb_s: f32 = @as(f32, @floatFromInt(self.input_file_size / 1_048_576)) / (@as(f32, @floatFromInt(time_taken_us)) / 1_000_000);

        std.debug.print(
            "Compressed file from {d} to {d} bytes in {d}ms\n", 
            .{self.input_file_size, self.compressed_file_size, @divFloor(time_taken_us, 1000)}
        );
        std.debug.print("{d}MB/s\n", .{mb_s});
    }
};

pub fn main() !void {
    const filename = "../../data/enwik8";

    var arena = std.heap.ArenaAllocator.init(std.heap.page_allocator);
    defer arena.deinit();

    var stream = try BitStream.init(filename, arena.allocator());
    defer stream.deinit();

    try stream.compress(arena.allocator());
}
