system = Module("system"
print = system.out.write_line
io = Module("io"
assert = @(x) x || throw Throwable("Assertion failed."
$assert_equals = @(actual, expected)
	print(actual
	assert(actual == expected
Source = Object + @
	$close = @
	$read
	$__initialize = @(list)
		n = list.size(
		i = 0
		$read = @(buffer, offset, size)
			size > n && (size = n)
			for j = 0; j < size; j = j + 1
				buffer[offset] = list[i]
				:i = i + 1
				offset = offset + 1
			:n = n - size
			size
$Source = Source
$list_to_string = @(list) io.Reader(Source(list), "utf-8").read(list.size(
