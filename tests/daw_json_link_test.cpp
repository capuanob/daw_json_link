﻿// The MIT License (MIT)
//
// Copyright (c) 2017 Darrell Wright
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files( the "Software" ), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <cstdint>
#include <cstdlib>
#include <iostream>

#include <daw/daw_benchmark.h>
#include <daw/daw_memory_mapped_file.h>

#include "daw_json_link.h"
#include <codecvt>

struct A : public daw::json::daw_json_link<A> {
	int a;
	double b;
	bool c;
	std::string d;

	static void json_link_map( ) {
		link_json_integer_fn( "a", []( A &obj, int value ) { obj.a = std::move( value ); },
		                      []( A const &obj ) { return obj.a; } );
		link_json_real_fn( "b", []( A &obj, double value ) { obj.b = std::move( value ); },
		                   []( A const &obj ) { return obj.b; } );
		link_json_boolean_fn( "c", []( A &obj, bool value ) { obj.c = std::move( value ); },
		                      []( A const &obj ) { return obj.c; } );
		link_json_string_fn( "d", []( A &obj, std::string value ) { obj.d = std::move( value ); },
		                     []( A const &obj ) { return obj.d; } );
	}
};
struct A2 {
	int a;
	double b;
	bool c;
	std::string d;
};

struct B : public daw::json::daw_json_link<B> {
	A a;

	static void json_link_map( ) {
		link_json_object_fn( "a", []( B &obj, A value ) { obj.a = std::move( value ); },
		                     []( B const &obj ) { return obj.a; } );
	}
};

auto make_path_str( std::string s ) {
#ifdef WIN32
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
	return converter.from_bytes( s );
}
#else
	return std::move( s );
}
#endif

int main( int argc, char **argv ) {
	std::cout << "Size of linked class->" << sizeof( A ) << " vs size of unlinked->" << sizeof( A2 ) << '\n';
	boost::string_view str = "{ \"a\": { \"a\" : 5, \"b\" : 6.6, \"c\" : true, \"d\": \"hello\" }}";
	std::string const str_array = "[" + str.to_string( ) + "," + str.to_string( ) + "]";
	auto a = B::from_json_string( str.begin( ), str.end( ) ).result;
	std::cout << a.to_json_string( ) << '\n';

	std::cout << "Attemping json array '" << str_array << "'\n[";
	bool is_first = true;
	auto c = B::from_json_array_string( str_array.data( ), str_array.data( ) + str_array.size( ) );

	for( auto const &item : c ) {
		if( !is_first ) {
			std::cout << ",";
		} else {
			is_first = false;
		}
		std::cout << item.to_json_string( ) << "]\n";
	}
	if( boost::filesystem::exists( make_path_str( "test.json" ).data( ) ) ) {
		daw::filesystem::MemoryMappedFile<char> json_file{make_path_str( "test.json" ).data( )};
		daw::exception::daw_throw_on_false( json_file, "Failed to open test file 'test.json'" );
		auto lapsed_time = daw::benchmark( [&json_file]( ) {
			B::from_json_array_string( json_file.data( ), json_file.data( ) + json_file.size( ) );
		} );
		std::cout << "To process " << json_file.size( ) << " bytes, it took " << lapsed_time << " seconds\n";
	}
	return EXIT_SUCCESS;
}