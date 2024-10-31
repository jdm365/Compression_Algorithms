const std = @import("std");


const BUFFER_SIZE: usize = (1 << 20);
var SCRATCH_BUFFER: [4096]u8 = undefined;


const HuffmanNode = struct {
    value: u8,
    freq: u32,
    left:  *HuffmanNode,
    right: *HuffmanNode,
};

pub fn buildHuffmanTree(
    allocator: std.mem.Allocator,
    buffer: *[BUFFER_SIZE]u8,
    root: *HuffmanNode,
) !void {
    // To start, do this on a chunk by chunk level.
    var freqs: [256]usize = undefined;
    @memset(freqs[0..256], 0);

    var idx: usize = 0;
    while (idx < BUFFER_SIZE) : (idx += 4) {
        const idx_0: usize = @intCast(buffer[idx]);
        const idx_1: usize = @intCast(buffer[idx+1]);
        const idx_2: usize = @intCast(buffer[idx+2]);
        const idx_3: usize = @intCast(buffer[idx+3]);

        freqs[idx_0] += 1;
        freqs[idx_1] += 1;
        freqs[idx_2] += 1;
        freqs[idx_3] += 1;
    }

    // TODO: Build huffman context to pass into pq.
    const PQlt = std.PriorityQueue(*HuffmanNode, void, std.mem.lessThan);
    var pq = PQlt.init(allocator, {});
    defer pq.deinit();

    for (0.., freqs[0..256]) |i, freq| {
        if (freq > 0) {
            const new_node = HuffmanNode{
                .value = @intCast(i),
                .freq = freq,
                .left = null,
                .right = null,
            };
            try pq.add(&new_node);
        }
    }

    while (pq.count() > 1) {
        const left  = pq.remove();
        const right = pq.remove();

        const parent = HuffmanNode{
            .value = 0,
            .freq = left.freq + right.freq,
            .left = left,
            .right = right,
        };

        try pq.add(&parent);
    }

    root = pq.remove();
}


const BitStream = struct {
    input_file: std.fs.File,
    output_file: std.fs.File,
    buffer: [BUFFER_SIZE]u8,
    input_file_size: usize,
    buffer_idx: usize,
    file_byte_idx: usize,
    file_bit_idx: usize,

    pub fn init(
        input_filename: []const u8,
    ) !BitStream {
        @memcpy(SCRATCH_BUFFER[0..input_filename.len], input_filename);
        @memcpy(SCRATCH_BUFFER[input_filename.len..input_filename.len+4], ".fse");

        const input_file  = try std.fs.cwd().openFile(input_filename, .{});
        const output_file = try std.fs.cwd().createFile(
            SCRATCH_BUFFER[0..input_filename.len+4], 
            .{ .read = true },
            );
        const input_file_size = try input_file.getEndPos();

        return BitStream{
            .input_file = input_file,
            .output_file = output_file,
            .buffer = undefined,
            .input_file_size = input_file_size,
            .buffer_idx = 0,
            .file_byte_idx = 0,
            .file_bit_idx = 0,
        };
    }

    pub fn deinit(self: *BitStream) void {
        self.input_file.close();
        self.output_file.close();
    }

    pub fn flushChunk(self: *BitStream) !void {
        _ = try self.output_file.write(
            self.buffer[0..self.buffer_idx]
            );
        self.buffer_idx = 0;
    }

    pub fn readChunk(self: *BitStream) !void {
        const bytes_remaining = self.input_file_size - self.file_byte_idx;
        const bytes_to_read = @min(bytes_remaining, BUFFER_SIZE);
        _ = try self.input_file.read(
            self.buffer[0..bytes_to_read]
            );
        self.file_byte_idx += bytes_to_read;
    }

    pub fn compress(self: *BitStream, allocator: std.mem.Allocator) !void {
        while (self.file_byte_idx < self.input_file_size) {
            try self.readChunk();
            const root: *HuffmanNode = undefined;
            try buildHuffmanTree(allocator, &self.buffer, root);
            try self.flushChunk();
        }

        std.debug.print(
            "Compressed file from {d} to {d} bytes\n", 
            .{self.input_file_size, self.file_byte_idx}
        );
    }
};

pub fn main() !void {
    const filename = "../../data/enwik8";

    var arena = std.heap.ArenaAllocator.init(std.heap.page_allocator);
    defer arena.deinit();

    var stream = try BitStream.init(filename);
    defer stream.deinit();

    try stream.compress(arena.allocator());
}
