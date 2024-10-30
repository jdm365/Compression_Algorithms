const std = @import("std");


const BUFFER_SIZE: usize = (1 << 20);
var SCRATCH_BUFFER: [4096]u8 = undefined;


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
            // std.mem.sliceAsBytes(self.buffer)
            self.buffer[0..self.buffer_idx]
            );
        self.buffer_idx = 0;
    }

    pub fn readChunk(self: *BitStream) !void {
        const bytes_remaining = self.input_file_size - self.file_byte_idx;
        const bytes_to_read = @min(bytes_remaining, BUFFER_SIZE);
        _ = try self.input_file.read(
            // std.mem.sliceAsBytes(self.buffer[self.file_byte_idx..self.file_byte_idx+bytes_to_read])
            self.buffer[0..bytes_to_read]
            );
        self.file_byte_idx += bytes_to_read;
    }

    pub fn compress(self: *BitStream) !void {
        while (self.file_byte_idx < self.input_file_size) {
            try self.readChunk();
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

    // var arena = std.heap.ArenaAllocator(std.heap.page_allocator);
    // defer arena.deinit();

    var stream = try BitStream.init(filename);
    defer stream.deinit();

    try stream.compress();
}
