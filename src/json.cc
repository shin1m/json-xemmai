#include <xemmai/convert.h>
#include <format>

namespace xemmaix::json
{

using namespace xemmai;

struct t_library : xemmai::t_library
{
	using xemmai::t_library::t_library;
	XEMMAI__LIBRARY__MEMBERS
};

XEMMAI__LIBRARY__BASE(t_library, t_global, f_global())

void t_library::f_scan(t_scan a_scan)
{
}

template<typename T_source>
struct t_parser
{
	T_source& v_source;
	size_t v_line = 1;
	size_t v_column = 1;
	decltype(v_source.f_get()) v_c;

	void f_throw [[noreturn]] ()
	{
		throw std::runtime_error(std::format("invalid 0x{:x} at {}:{}\n", v_c, v_line, v_column));
	}
	void f_get()
	{
		switch (v_c) {
		case '\n':
			++v_line;
			v_column = 1;
			break;
		default:
			++v_column;
		}
		v_c = v_source.f_get();
	}
	void f_next()
	{
		while (std::isspace(v_c)) f_get();
	}
	t_object* f_string();
	t_parser(T_source& a_source) : v_source(a_source), v_c(v_source.f_get())
	{
	}
	t_pvalue f_value();
};

template<typename T_source>
t_object* t_parser<T_source>::f_string()
{
	f_get();
	std::vector<wchar_t> value;
	while (v_c != -1) {
		if (v_c == '"') {
			f_get();
			break;
		} else if (v_c == '\\') {
			f_get();
			switch (v_c) {
			case '"':
				value.push_back('"');
				break;
			case '\\':
				value.push_back('\\');
				break;
			case '/':
				value.push_back('/');
				break;
			case 'b':
				value.push_back('\b');
				break;
			case 'f':
				value.push_back('\f');
				break;
			case 'n':
				value.push_back('\n');
				break;
			case 'r':
				value.push_back('\r');
				break;
			case 't':
				value.push_back('\t');
				break;
			case 'u':
				{
					auto code = [&]
					{
						auto hex = [&]
						{
							f_get();
							if (std::isdigit(v_c)) return v_c - '0';
							if (v_c >= 'A' && v_c <= 'F') return v_c - 'A' + 10;
							if (v_c >= 'a' && v_c <= 'f') return v_c - 'a' + 10;
							f_throw();
						};
						auto c = hex() << 12;
						c += hex() << 8;
						c += hex() << 4;
						return c + hex();
					};
					auto c = code();
					if (c < 0xd800 || c >= 0xe000) {
						value.push_back(c);
					} else {
						if (c >= 0xdc00) f_throw();
						f_get();
						if (v_c != '\\') f_throw();
						f_get();
						if (v_c != 'u') f_throw();
						auto d = code();
						if (d < 0xdc00 || d >= 0xe000) f_throw();
						value.push_back(0x10000 + (c - 0xd800) * 0x400 + d - 0xdc00);
					}
				}
				break;
			default:
				f_throw();
			}
		} else {
			value.push_back(v_source.f_wide(v_c));
		}
		f_get();
	}
	return t_string::f_instantiate(value.data(), value.size());
}

template<typename T_source>
t_pvalue t_parser<T_source>::f_value()
{
	f_next();
	switch (v_c) {
	case '"':
		return f_string();
	case '[':
		{
			f_get();
			auto p = t_list::f_instantiate();
			auto& list = p->f_as<t_list>();
			f_next();
			if (v_c != ']') {
				while (true) {
					list.f_push(f_value());
					f_next();
					if (v_c != ',') break;
					f_get();
				}
				if (v_c != ']') f_throw();
			}
			f_get();
			return p;
		}
	case '{':
		{
			f_get();
			auto p = t_map::f_instantiate();
			auto& map = p->f_as<t_map>();
			f_next();
			if (v_c != '}') {
				while (true) {
					auto key = f_string();
					f_next();
					if (v_c != ':') f_throw();
					f_get();
					map.f_put(key, f_value());
					f_next();
					if (v_c != ',') break;
					f_get();
					f_next();
				}
				if (v_c != '}') f_throw();
			}
			f_get();
			return p;
		}
	case 'f':
		f_get();
		if (v_c != 'a') f_throw();
		f_get();
		if (v_c != 'l') f_throw();
		f_get();
		if (v_c != 's') f_throw();
		f_get();
		if (v_c != 'e') f_throw();
		f_get();
		return false;
	case 'n':
		f_get();
		if (v_c != 'u') f_throw();
		f_get();
		if (v_c != 'l') f_throw();
		f_get();
		if (v_c != 'l') f_throw();
		f_get();
		return nullptr;
	case 't':
		f_get();
		if (v_c != 'r') f_throw();
		f_get();
		if (v_c != 'u') f_throw();
		f_get();
		if (v_c != 'e') f_throw();
		f_get();
		return true;
	}
	if (v_c != '-' && !std::isdigit(v_c)) f_throw();
	char cs[32];
	char* p = cs;
	auto get = [&]
	{
		if (p >= cs + sizeof(cs)) f_throw();
		*p++ = v_c;
		f_get();
	};
	do get(); while (std::isdigit(v_c));
	if (v_c == '.') {
		do get(); while (std::isdigit(v_c));
		if (v_c == 'E' || v_c == 'e') {
			get();
			if (v_c == '+' || v_c == '-') get();
			if (!std::isdigit(v_c)) f_throw();
			do get(); while (std::isdigit(v_c));
		}
		double value;
		if (std::from_chars(cs, p, value).ec != std::errc()) f_throw();
		return value;
	} else {
		intptr_t value;
		if (std::from_chars(cs, p, value).ec != std::errc()) f_throw();
		return value;
	}
}

struct t_bytes_source
{
	t_pvalue v_read;
	t_object* v_buffer;
	size_t v_n = 0;
	size_t v_i = 0;

	int f_read();
	t_bytes_source(const t_pvalue& a_read) : v_read(a_read), v_buffer(t_bytes::f_instantiate(1024))
	{
	}
	int f_get()
	{
		return v_i < v_n ? f_as<const t_bytes&>(v_buffer)[v_i++] : f_read();
	}
	wchar_t f_wide(wchar_t a_c)
	{
		auto d =
			a_c < 0x80 ? 1 :
			a_c < 0xc0 ? 0 :
			a_c < 0xe0 ? 2 :
			a_c < 0xf0 ? 3 :
			a_c < 0xf8 ? 4 :
			a_c < 0xfc ? 5 :
			a_c < 0xfe ? 6 :
			0;
		if (d < 2) return a_c;
		a_c &= 0x7f >> d;
		do {
			a_c <<= 6;
			a_c |= f_get() & 0x3f;
		} while (--d > 1);
		return a_c;
	}
};

int t_bytes_source::f_read()
{
	auto& bytes = f_as<t_bytes&>(v_buffer);
	auto n = v_read(v_buffer, 0, bytes.f_size());
	f_check<size_t>(n, L"number of bytes");
	v_n = f_as<size_t>(n);
	v_i = 0;
	return v_i < v_n ? bytes[v_i++] : -1;
}

struct t_string_source
{
	const wchar_t* v_i;
	const wchar_t* v_j;

	t_string_source(const t_string& a_value) : v_i(a_value), v_j(v_i + a_value.f_size())
	{
	}
	wint_t f_get()
	{
		return v_i == v_j ? -1 : *v_i++;
	}
	wchar_t f_wide(wchar_t a_c)
	{
		return a_c;
	}
};

template<typename T_target>
struct t_generator
{
	T_target& v_target;
	size_t v_space;

	void f_put(std::string_view a_s)
	{
		for (auto c : a_s) v_target.f_put(c);
	}
	void f_indent(size_t a_depth)
	{
		if (v_space <= 0) return;
		v_target.f_put('\n');
		size_t n = v_space * a_depth;
		for (size_t i = 0; i < n; ++i) v_target.f_put(' ');
	}
	void f_string(std::wstring_view a_s);
	t_generator(T_target& a_target, size_t a_space) : v_target(a_target), v_space(a_space)
	{
	}
	void f_value(const t_pvalue& a_value, size_t a_depth);
};

template<typename T_target>
void t_generator<T_target>::f_string(std::wstring_view a_s)
{
	v_target.f_put('"');
	for (auto c : a_s)
		if (c < 0x20) {
			v_target.f_put('\\');
			switch (c) {
			case '\b':
				v_target.f_put('b');
				break;
			case '\f':
				v_target.f_put('f');
				break;
			case '\n':
				v_target.f_put('n');
				break;
			case '\r':
				v_target.f_put('r');
				break;
			case '\t':
				v_target.f_put('t');
				break;
			default:
				f_put("u00");
				auto hex = [&](auto c)
				{
					v_target.f_put(c < 10 ? c + '0' : c - 10 + 'A');
				};
				hex(c >> 4);
				hex(c & 0xf);
			}
		} else if (c == '"' || c == '\\') {
			v_target.f_put('\\');
			v_target.f_put(c);
		} else {
			v_target.f_wide(c);
		}
	v_target.f_put('"');
}

template<typename T_target>
void t_generator<T_target>::f_value(const t_pvalue& a_value, size_t a_depth)
{
	if (!a_value) {
		f_put("null"sv);
	} else if (a_value.f_tag() == c_tag__FALSE) {
		f_put("false"sv);
	} else if (a_value.f_tag() == c_tag__TRUE) {
		f_put("true"sv);
	} else if (f_is<intptr_t>(a_value)) {
		char cs[32];
		auto [p, ec] = std::to_chars(cs, cs + sizeof(cs), f_as<intptr_t>(a_value));
		if (ec != std::errc()) throw std::system_error(std::make_error_code(ec));
		f_put({cs, p});
	} else if (f_is<double>(a_value)) {
		char cs[32];
		auto [p, ec] = std::to_chars(cs, cs + sizeof(cs), f_as<double>(a_value));
		if (ec != std::errc()) throw std::system_error(std::make_error_code(ec));
		f_put({cs, p});
	} else if (f_is<t_string>(a_value)) {
		f_string(a_value->f_as<t_string>());
	} else if (f_is<t_list>(a_value)) {
		v_target.f_put('[');
		auto& list = a_value->f_as<t_list>();
		t_pvalue x;
		if (list.f_owned_or_shared<std::shared_lock>([&]
		{
			if (list.f_size() <= 0) return false;
			x = list[0];
			return true;
		})) {
			++a_depth;
			for (size_t i = 0;;) {
				f_indent(a_depth);
				f_value(x, a_depth);
				++i;
				if (!list.f_owned_or_shared<std::shared_lock>([&]
				{
					if (i >= list.f_size()) return false;
					x = list[i];
					return true;
				})) break;
				v_target.f_put(',');
			}
			f_indent(--a_depth);
		}
		v_target.f_put(']');
	} else if (f_is<t_map>(a_value)) {
		v_target.f_put('{');
		auto& map = a_value->f_as<t_map>();
		t_map::t_iterator i(map);
		t_pvalue x;
		t_pvalue y;
		auto get = [&]
		{
			if (!i.f_entry()) return false;
			x = i.f_entry()->f_key();
			y = i.f_entry()->v_value;
			return true;
		};
		if (map.f_owned_or_shared<t_shared_lock_with_safe_region>(get)) {
			++a_depth;
			while (true) {
				f_indent(a_depth);
				f_check<t_string>(x, L"key");
				f_string(x->f_as<t_string>());
				v_target.f_put(':');
				if (v_space) v_target.f_put(' ');
				f_value(y, a_depth);
				if (!map.f_owned_or_shared<t_shared_lock_with_safe_region>([&]
				{
					i.f_next();
					return get();
				})) break;
				v_target.f_put(',');
			}
			f_indent(--a_depth);
		}
		v_target.f_put('}');
	} else {
		throw std::runtime_error("unsupported value");
	}
}

struct t_bytes_target
{
	t_pvalue v_write;
	t_object* v_buffer;
	size_t v_n = 0;

	t_bytes_target(const t_pvalue& a_write) : v_write(a_write), v_buffer(t_bytes::f_instantiate(1024))
	{
	}
	~t_bytes_target()
	{
		if (v_n > 0) v_write(v_buffer, 0, v_n);
	}
	void f_put(char a_c)
	{
		auto& bytes = f_as<t_bytes&>(v_buffer);
		if (v_n >= bytes.f_size()) {
			v_write(v_buffer, 0, v_n);
			v_n = 0;
		}
		bytes[v_n++] = a_c;
	}
	void f_wide(wchar_t a_c)
	{
		if (a_c < L'\u0080') {
			f_put(a_c);
		} else {
			auto d =
				a_c < L'\u0800' ? 1 :
				a_c < L'\U00010000' ? 2 :
				a_c < wchar_t(0x200000) ? 3 :
				a_c < wchar_t(0x4000000) ? 4 :
				5;
			auto s = d * 6;
			f_put(a_c >> s | int8_t(0x80) >> d);
			do {
				s -= 6;
				f_put(a_c >> s & 0x3f | 0x80);
			} while (s > 0);
		}
	}
};

struct t_string_target : t_stringer
{
	t_object* f_get() const
	{
		return *this;
	}
	void f_put(wchar_t a_c)
	{
		*this << a_c;
	}
	void f_wide(wchar_t a_c)
	{
		f_put(a_c);
	}
};

std::vector<std::pair<t_root, t_rvalue>> t_library::f_define()
{
	return t_define(this)
		(L"parse"sv, t_static<t_pvalue(*)(const t_pvalue&), [](auto a_read)
		{
			t_bytes_source source(a_read);
			return t_parser(source).f_value();
		}>())
		(L"generate"sv, t_static<void(*)(const t_pvalue&, size_t, const t_pvalue&), [](auto a_value, auto a_space, auto a_write)
		{
			t_bytes_target target(a_write);
			t_generator(target, a_space).f_value(a_value, 0);
		}>())
		(L"build"sv, t_static<t_pvalue(*)(const t_string&), [](auto a_value)
		{
			t_string_source source(a_value);
			return t_parser(source).f_value();
		}>())
		(L"stringify"sv, t_static<t_object*(*)(const t_pvalue&, size_t), [](auto a_value, auto a_space)
		{
			t_string_target target;
			t_generator(target, a_space).f_value(a_value, 0);
			return target.f_get();
		}>())
	;
}

}

XEMMAI__MODULE__FACTORY(xemmai::t_library::t_handle* a_handle)
{
	return xemmai::f_new<xemmaix::json::t_library>(a_handle);
}
