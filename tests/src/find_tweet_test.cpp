// Copyright (c) Darrell Wright
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/beached/daw_json_link
//

#include "defines.h"

#include "daw/json/daw_json_iterator.h"
#include "daw/json/daw_json_link.h"

#include <daw/daw_benchmark.h>
#include <daw/daw_read_file.h>

#include <iostream>
#include <tuple>

struct tweet {
	std::uint64_t id;
	std::string_view text;
};

namespace daw::json {
	template<>
	struct json_data_contract<tweet> {
		static constexpr char const id[] = "id_str";
		static constexpr char const text[] = "text";

		using type = json_member_list<
		  json_number<id, std::uint64_t,
		              options::number_opt( options::LiteralAsStringOpt::Always )>,
		  json_link<text, std::string_view>>;

		static constexpr auto to_json_data( tweet const &t ) {
			return std::forward_as_tuple( t.id, t.text );
		}
	};
} // namespace daw::json

#if not defined( DAW_NUM_RUNS )
#if not defined( DEBUG ) or defined( NDEBUG )
static inline constexpr std::size_t DAW_NUM_RUNS = 250;
#else
static inline constexpr std::size_t DAW_NUM_RUNS = 2;
#endif
#endif
static_assert( DAW_NUM_RUNS > 0 );

template<daw::json::options::ExecModeTypes ExecMode>
void test( std::string_view json_sv1, std::uint64_t id ) {
	std::cout << "Using " << to_string( ExecMode )
	          << " exec model\n*********************************************\n";

	auto const sz = json_sv1.size( );
	{
		using range_t = daw::json::json_array_range<tweet, ExecMode>;
		auto result = tweet{ };
		auto res = daw::bench_n_test_mbs<DAW_NUM_RUNS>(
		  "find_tweet bench(checked)", sz,
		  [&]( auto rng ) {
			  for( tweet t : rng ) {
				  if( t.id == id ) {
					  result = t;
					  return;
				  }
			  }
			  result = tweet{ };
		  },
		  range_t( json_sv1, "statuses" ) );
		test_assert( result.id == id,
		             "Exception while parsing: res.get_exception_message()" );
		std::cout << "found tweet id: " << id << '\n'
		          << daw::json::to_json( result ) << '\n';
	}
	{
		using range_t = daw::json::json_array_range<
		  tweet, daw::json::options::CheckedParseMode::no, ExecMode>;
		auto result = tweet{ };
		auto res = daw::bench_n_test_mbs<DAW_NUM_RUNS>(
		  "find_tweet bench(unchecked)", sz,
		  [&]( auto rng ) {
			  for( tweet t : rng ) {
				  if( t.id == id ) {
					  result = t;
					  return;
				  }
			  }
			  result = tweet{ };
		  },
		  range_t( json_sv1, "statuses" ) );
		test_assert( result.id == id,
		             "Exception while parsing: res.get_exception_message()" );
		std::cout << "found tweet id: " << id << '\n'
		          << daw::json::to_json( result ) << '\n';
	}
	{
		auto result = tweet{ };
		using namespace daw::json;
		auto res = daw::bench_n_test_mbs<DAW_NUM_RUNS>(
		  "find_tweet bench(checked, json_value)", sz,
		  [&]( auto sv ) {
			  for( auto jp : json_value( sv )["statuses"] ) {
				  auto const cur_id =
				    jp.value["id_str"]
				      .as<json_number_no_name<
				        std::uint64_t,
				        options::number_opt( options::LiteralAsStringOpt::Always )>>( );
				  if( cur_id == id ) {
					  result = from_json<tweet>( jp.value );
					  return;
				  }
			  }
			  result = tweet{ };
		  },
		  json_sv1 );
		test_assert( result.id == id,
		             "Exception while parsing: res.get_exception_message()" );
		std::cout << "found tweet id: " << id << '\n'
		          << daw::json::to_json( result ) << '\n';
	}
	{
		auto result = tweet{ };
		using namespace daw::json;
		auto res = daw::bench_n_test_mbs<DAW_NUM_RUNS>(
		  "find_tweet bench(unchecked, json_value)", sz,
		  [&]( auto sv ) {
			  for( auto jp :
			       basic_json_value<parse_options( options::CheckedParseMode::no )>(
			         sv )["statuses"] ) {
				  auto const cur_id =
				    jp.value["id_str"]
				      .as<json_number_no_name<
				        std::uint64_t,
				        options::number_opt( options::LiteralAsStringOpt::Always )>>( );
				  if( cur_id == id ) {
					  result = from_json<tweet>(
					    jp.value, options::parse_flags<options::CheckedParseMode::no> );
					  return;
				  }
			  }
			  result = tweet{ };
		  },
		  json_sv1 );
		test_assert( result.id == id,
		             "Exception while parsing: res.get_exception_message()" );
		std::cout << "found tweet id: " << id << '\n'
		          << daw::json::to_json( result ) << '\n';
	}
}

int main( int argc, char **argv )
#ifdef DAW_USE_EXCEPTIONS
  try
#endif
{

	using namespace daw::json;
	if( argc < 2 ) {
		std::cerr << "Must supply a filenames to open(twitter_timeline.json)\n";
		exit( 1 );
	}

	auto const json_data1 = *daw::read_file( argv[1] );
	auto const json_sv1 =
	  std::string_view( json_data1.data( ), json_data1.size( ) );
	assert( json_sv1.size( ) > 2 and "Minimum json data size is 2 '{}'" );

	auto const sz = json_sv1.size( );
	std::cout << "Processing: " << daw::utility::to_bytes_per_second( sz )
	          << '\n';

	constexpr std::uint64_t id = 505874901689851904ULL; // 144179654289408000ULL;
	test<options::ExecModeTypes::compile_time>( json_sv1, id );
}
#ifdef DAW_USE_EXCEPTIONS
catch( daw::json::json_exception const &jex ) {
	std::cerr << "Exception thrown by parser: " << jex.reason( ) << '\n';
	exit( 1 );
} catch( std::exception const &ex ) {
	std::cerr << "Unknown exception thrown during testing: " << ex.what( )
	          << '\n';
	exit( 1 );
} catch( ... ) {
	std::cerr << "Unknown exception thrown during testing\n";
	throw;
}
#endif
