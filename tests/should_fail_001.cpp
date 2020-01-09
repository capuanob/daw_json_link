// The MIT License (MIT)
//
// Copyright (c) 2019 Darrell Wright
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

// Ensure that we are always checking
#ifndef DAW_JSON_CHECK_DEBUG_ONLY
#define DAW_JSON_CHECK_DEBUG_ONLY
#endif

#include <daw/json/daw_json_link.h>

#include <iostream>
#include <optional>
#include <string_view>

namespace tests {
	struct Coordinate {
		double lat;
		double lng;
		std::optional<std::string> name;
	};

	struct UriList {
		std::vector<std::string> uris;
	};
} // namespace tests

template<>
struct daw::json::json_data_contract<tests::Coordinate> {
#ifdef __cpp_nontype_template_parameter_class
	using type = json_member_list<json_number<"lat">, json_number<"lng">,
	                              json_string<"name">>;
#else
	static inline constexpr char const lat[] = "lat";
	static inline constexpr char const lng[] = "lng";
	static inline constexpr char const name[] = "name";
	using type =
	  json_member_list<json_number<lat>, json_number<lng>, json_string<name>>;
#endif
};

template<>
struct daw::json::json_data_contract<tests::UriList> {
#ifdef __cpp_nontype_template_parameter_class
	using type = json_member_list<json_array<"uris", std::string>>;
#else
	static inline constexpr char const uris[] = "uris";
	using type = json_member_list<json_array<uris, std::string>>;
#endif
};

namespace tests {
	bool quotes_in_numbers( ) {
		static constexpr std::string_view data =
		  R"({"lat": "55.55", "lng": "12.34" })";
		try {
			Coordinate c = daw::json::from_json<tests::Coordinate>( data );
			(void)c;
		} catch( daw::json::json_exception const & ) { return true; }
		return false;
	}

	bool bool_in_numbers( ) {
		static constexpr std::string_view data = R"({"lat": true, "lng": false })";
		try {
			Coordinate c = daw::json::from_json<tests::Coordinate>( data );
			(void)c;
		} catch( daw::json::json_exception const & ) { return true; }
		return false;
	}

	bool invalid_numbers( ) {
		static constexpr std::string_view data =
		  R"({"lat": 1.23b34, "lng": 1234.4 })";
		try {
			Coordinate c = daw::json::from_json<tests::Coordinate>( data );
			(void)c;
		} catch( daw::json::json_exception const & ) { return true; }
		return false;
	}

	bool missing_value_001( ) {
		static constexpr std::string_view data = R"({"lat": 1.23, "lng": })";
		try {
			Coordinate c = daw::json::from_json<tests::Coordinate>( data );
			(void)c;
		} catch( daw::json::json_exception const & ) { return true; }
		return false;
	}

	bool missing_value_002( ) {
		static constexpr std::string_view data = R"({"lat": , "lng": 1.23 })";
		try {
			Coordinate c = daw::json::from_json<tests::Coordinate>( data );
			(void)c;
		} catch( daw::json::json_exception const & ) { return true; }
		return false;
	}

	bool missing_value_003( ) {
		static constexpr std::string_view data =
		  R"({"name": "lat": 1.23, "lng": 1.34 })";
		try {
			Coordinate c = daw::json::from_json<tests::Coordinate>( data );
			(void)c;
		} catch( daw::json::json_exception const & ) { return true; }
		return false;
	}

	bool missing_member( ) {
		static constexpr std::string_view data = R"({"lng": 1.23 })";
		try {
			Coordinate c = daw::json::from_json<tests::Coordinate>( data );
			(void)c;
		} catch( daw::json::json_exception const & ) { return true; }
		return false;
	}

	bool missing_closing_brace( ) {
		static constexpr std::string_view data = R"({"lng": 1.23, "lat": 1.22 )";
		try {
			Coordinate c = daw::json::from_json<tests::Coordinate>( data );
			(void)c;
		} catch( daw::json::json_exception const & ) { return true; }
		return false;
	}

	bool invalid_strings( ) {
		std::string data =
		  R"({"uris": [ "http://www.example.com", "http://www.example.com/missing_quote ] })";
		try {
			UriList ul = daw::json::from_json<tests::UriList>( data );
			(void)ul;
		} catch( daw::json::json_exception const & ) { return true; }
		return false;
	}

	bool missing_array_element( ) {
		std::string data = "[1,2,,3]";
		try {
			std::vector<int> numbers = daw::json::from_json_array<int>( data );
			(void)numbers;
		} catch( daw::json::json_exception const & ) { return true; }
		return false;
	}

	bool very_large_signed_int( ) {
		std::string data =
		  "[123, 32767, 32768, -237462374673276894279832749832423479823246327846, "
		  "55 ]";
		try {
			using namespace daw::json;
			std::vector<intmax_t> numbers =
			  from_json_array<json_checked_number<no_name, intmax_t>>( data );
			(void)numbers;
		} catch( daw::json::json_exception const & ) { return true; }
		return false;
	}

	bool incomplete_null( ) {
		std::string data = "[nul]";
		try {
			using namespace daw::json;
			std::vector<std::optional<int>> numbers =
			  from_json_array<json_checked_number_null<no_name, std::optional<int>>>(
			    data );
			(void)numbers;
		} catch( daw::json::json_exception const & ) { return true; }
		return false;
	}

	bool incomplete_false( ) {
		std::string data = "[fa]";
		try {
			using namespace daw::json;
			std::vector<bool> bools = from_json_array<bool>( data );
			(void)bools;
		} catch( daw::json::json_exception const & ) { return true; }
		return false;
	}

	bool incomplete_true( ) {
		std::string data = "[tr]";
		try {
			using namespace daw::json;
			std::vector<bool> bools = from_json_array<bool>( data );
			(void)bools;
		} catch( daw::json::json_exception const & ) { return true; }
		return false;
	}

	bool bad_true( ) {
		std::string data = "[tree]";
		try {
			using namespace daw::json;
			std::vector<bool> bools = from_json_array<bool>( data );
			(void)bools;
		} catch( daw::json::json_exception const & ) { return true; }
		return false;
	}
} // namespace tests

#define expect_fail( Bool, Reason )                                            \
	while( not static_cast<bool>( Bool ) ) {                                     \
		std::cerr << "Fail: " << Reason << '\n';                                   \
		exit( 1 );                                                                 \
	}                                                                            \
	while( false )

int main( ) {
	expect_fail( tests::quotes_in_numbers( ),
	             "Failed to find unexpected quotes in numbers" );
	expect_fail( tests::bool_in_numbers( ),
	             "Failed to find a bool when a number was expected" );
	expect_fail( tests::invalid_numbers( ), "Failed to find an invalid number" );
	expect_fail( tests::invalid_strings( ), "Failed to find missing quote" );

	expect_fail( tests::missing_array_element( ),
	             "Failed to catch an empty array element e.g(1,,2)" );

	expect_fail( tests::missing_value_001( ),
	             "Failed to catch a missing value that has a member name" );

	expect_fail( tests::missing_value_002( ),
	             "Failed to catch a missing value that has a member name" );

	expect_fail( tests::missing_value_003( ),
	             "Failed to catch a missing value that has a member name" );

	expect_fail( tests::missing_member( ),
	             "Failed to catch a missing required member" );

	expect_fail( tests::missing_closing_brace( ),
	             "Failed to catch a missing closing brace on object" );

	expect_fail( tests::very_large_signed_int( ),
	             "Failed to catch a very large signed number" );

	expect_fail( tests::incomplete_null( ),
	             "Incomplete null in array not caught" );

	expect_fail( tests::incomplete_true( ),
	             "Incomplete true in array not caught" );

	expect_fail( tests::incomplete_false( ),
	             "Incomplete false in array not caught" );

	expect_fail( tests::bad_true( ), "Incomplete true in array not caught" );
}