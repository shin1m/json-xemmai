json = Module("json"
utilities = Module("utilities"
assert_equals = utilities.assert_equals
list_to_string = utilities.list_to_string

generate = @(value, space)
	list = [
	json.generate(value, space, @(buffer, offset, size)
		while size > 0
			list.push(buffer[offset]
			offset = offset + 1
			size = size - 1
	list_to_string(list
test = @(space, value, text)
	assert_equals(generate(value, space), text
	assert_equals(json.stringify(value, space), text

test(0, null, "null"
test(2, null, "null"
test(0, false, "false"
test(2, false, "false"
test(0, true, "true"
test(2, true, "true"
test(0, 0, "0"
test(2, 0, "0"
test(0, -1, "-1"
test(2, -1, "-1"
test(0, 0.5, "0.5"
test(2, 0.5, "0.5"
test(0, "", "\"\""
test(2, "", "\"\""
test(0, "foo\n\0bar", "\"foo\\n\\u0000bar\""
test(2, "foo\n\0bar", "\"foo\\n\\u0000bar\""
test(0, [], "[]"
test(2, [], "[]"
test(0, [0], "[0]"
test(2, [0], "[
  0
]"
test(0, [0, 1], "[0,1]"
test(2, [0, 1], "[
  0,
  1
]"
test(0, {}, "{}"
test(2, {}, "{}"
test(0, {"foo": 0}, "{\"foo\":0}"
test(2, {"foo": 0}, "{
  \"foo\": 0
}"
test(0, {"foo": 0, "bar": 1}, "{\"foo\":0,\"bar\":1}"
test(2, {"foo": 0, "bar": 1}, "{
  \"foo\": 0,
  \"bar\": 1
}"
test(0, {"foo": [{}], "bar": {"zot": []}}, "{\"foo\":[{}],\"bar\":{\"zot\":[]}}"
test(2, {"foo": [{}], "bar": {"zot": []}}, "{
  \"foo\": [
    {}
  ],
  \"bar\": {
    \"zot\": []
  }
}"
test(0, list_to_string([
	0x24,
	0xc2, 0xa3,
	0xe0, 0xa4, 0xb9,
	0xe2, 0x82, 0xac,
	0xf0, 0x90, 0x8d, 0x88
]), list_to_string([
	0x22,
	0x24,
	0xc2, 0xa3,
	0xe0, 0xa4, 0xb9,
	0xe2, 0x82, 0xac,
	0xf0, 0x90, 0x8d, 0x88,
	0x22
])
