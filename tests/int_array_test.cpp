// The MIT License (MIT)
//
// Copyright (c) 2018-2019 Darrell Wright
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files( the "Software" ), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and / or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <fstream>
#include <iostream>
#include <vector>

#include <daw/daw_benchmark.h>
#include <daw/daw_do_n.h>
#include <daw/daw_random.h>

#include "daw/json/daw_json_link.h"

struct Number {
	intmax_t a{};
};

auto describe_json_class( Number ) noexcept {
	using namespace daw::json;
	return json_parser_t<json_number<"a", intmax_t>>{};
}

int main( ) {
	using namespace daw::json;
	constexpr size_t const NUMVALUES = 1'000'000;
	std::string const json_data = [] {
		std::string result = "[";

		result.reserve( NUMVALUES * ( 23 ) + 8 );
		daw::algorithm::do_n( NUMVALUES, [&result] {
			result += "{\"a\":" +
			          std::to_string( daw::randint<intmax_t>(
			            std::numeric_limits<intmax_t>::min( ),
			            std::numeric_limits<intmax_t>::max( ) ) ) +
			          "},";
		} );
		result.back( ) = ']';
		return result;
	}( );
	auto json_sv = daw::string_view( json_data );
	auto const count =
	  *daw::bench_n_test<4>( "int parsing 1", []( auto &&sv ) noexcept {
		  auto const data = from_json_array<json_class<"", Number>>( sv );
		  return data.size( );
	  },
	                         json_sv );

	std::cout << "element count: " << count << '\n';

	using iterator_t = daw::json::json_array_iterator<json_class<"", Number>>;

	auto data = std::vector<Number>( );

	auto const count2 = *daw::bench_n_test<4>(
	  "int parsing 2",
	  [&]( auto &&sv ) noexcept {
		  data.clear( );
		  std::copy( iterator_t( sv ), iterator_t( ), std::back_inserter( data ) );
		  return data.size( );
	  },
	  json_sv );

	std::cout << "element count 2: " << count2 << '\n';
}