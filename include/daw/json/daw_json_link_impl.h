﻿// The MIT License( MIT )
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

#pragma once

#include <array>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <iterator>
#include <limits>
#include <string_view>
#include <variant>

#include <daw/daw_algorithm.h>
#include <daw/daw_array.h>
#include <daw/daw_bounded_string.h>
#include <daw/daw_cxmath.h>
#include <daw/daw_exception.h>
#include <daw/daw_parser_helper_sv.h>
#include <daw/daw_string_view.h>
#include <daw/daw_traits.h>
#include <daw/daw_utility.h>
#include <daw/iso8601/daw_date_formatting.h>
#include <daw/iso8601/daw_date_parsing.h>
#include <daw/iterator/daw_back_inserter.h>

namespace daw {
	namespace json {
		namespace to_strings {
			namespace {
				using std::to_string;
			}
			template<typename T>
			auto to_string( std::optional<T> const &v )
			  -> decltype( to_string( *v ) ) {
				if( !v ) {
					return {"null"};
				}
				return to_string( *v );
			}
		} // namespace to_strings

		template<typename T>
		using is_a_json_type_detect = typename T::i_am_a_json_type;

		template<typename T>
		inline constexpr bool is_a_json_type_v =
		  daw::is_detected_v<is_a_json_type_detect, T>;

		template<typename T>
		bool is_null( std::optional<T> const &v ) {
			return !static_cast<bool>( v );
		}

		template<typename T>
		bool is_null( T const & ) {
			return false;
		}

		template<typename T>
		constexpr T from_json( std::string_view sv );

#if __cplusplus > 201703L or ( defined( __GNUC__ ) and __GNUC__ >= 9 )
		// C++ 20 Non-Type Class Template Arguments
#define JSONNAMETYPE daw::bounded_string

		template<typename String>
		constexpr size_t json_name_len( String const &str ) noexcept {
			return str.size( );
		}

		template<typename Lhs, typename Rhs>
		constexpr bool json_name_eq( Lhs const &lhs, Rhs const &rhs ) noexcept {
			return lhs == rhs;
		}
		// Convienience for array members
		static inline constexpr JSONNAMETYPE const no_name = "";
#else
#define JSONNAMETYPE char const *
		constexpr size_t json_name_len( char const *const str ) noexcept {
			return daw::string_view( str ).size( );
		}

		constexpr bool json_name_eq( char const *const lhs,
		                             char const *const rhs ) noexcept {
			return daw::string_view( lhs ) == daw::string_view( rhs );
		}

		// Convienience for array members
		static inline constexpr char const no_name[] = "";
#endif

		struct parse_js_date {
			constexpr std::chrono::time_point<std::chrono::system_clock,
			                                  std::chrono::milliseconds>
			operator( )( char const *ptr, size_t sz ) const {
				return daw::date_parsing::parse_javascript_timestamp(
				  daw::string_view( ptr, sz ) );
			}
		};

		template<typename T>
		struct custom_to_converter_t {
			constexpr decltype( auto ) operator( )( T &&value ) const {
				using std::to_string;
				return to_string( std::move( value ) );
			}
			constexpr decltype( auto ) operator( )( T const &value ) const {
				using std::to_string;
				return to_string( value );
			}
		};

		template<typename T>
		struct custom_from_converter_t {
			constexpr decltype( auto ) operator( )( std::string_view sv ) {
				return from_string( daw::tag<T>, sv );
			}
		};

		namespace impl {
			constexpr daw::string_view to_dsv( std::string_view const sv ) noexcept {
				return {sv.data( ), sv.size( )};
			}

			constexpr std::string_view to_ssv( daw::string_view const sv ) noexcept {
				return {sv.data( ), sv.size( )};
			}

			namespace data_size {
				constexpr char const *data( char const *ptr ) noexcept {
					return ptr;
				}

				constexpr size_t size( char const *ptr ) noexcept {
					return daw::string_view( ptr ).size( );
				}

				using std::data;
				template<typename T>
				using data_detect = decltype( data( std::declval<T &>( ) ) );

				using std::size;
				template<typename T>
				using size_detect = decltype( size( std::declval<T &>( ) ) );

				template<typename T>
				inline constexpr bool has_data_size_v =
				  daw::is_detected_v<data_detect, T>
				    and daw::is_detected_v<size_detect, T>;
			} // namespace data_size

			template<typename Container, typename OutputIterator,
			         daw::enable_if_t<daw::traits::is_container_like_v<
			           daw::remove_cvref_t<Container>>> = nullptr>
			constexpr OutputIterator copy_to_iterator( Container const &c,
			                                           OutputIterator it ) {
				for( auto const &value : c ) {
					*it++ = value;
				}
				return it;
			}

			template<typename OutputIterator>
			constexpr OutputIterator copy_to_iterator( char const *ptr,
			                                           OutputIterator it ) {
				if( ptr == nullptr ) {
					return it;
				}
				while( *ptr != '\0' ) {
					*it = *ptr;
					++it;
					++ptr;
				}
				return it;
			}

			template<typename T>
			using json_parser_description_t =
			  decltype( describe_json_class( std::declval<T &>( ) ) );

			template<typename T>
			constexpr auto deref_detect( T &&value ) noexcept -> decltype( *value ) {
				return *value;
			}

			template<typename Optional>
			using deref_t = decltype(
			  deref_detect( std::declval<daw::remove_cvref_t<Optional> &>( ) ) );

			template<typename Optional>
			inline constexpr bool is_valid_optional_v =
			  daw::is_detected_v<deref_t, Optional>;

			enum class JsonParseTypes : uint_fast8_t {
				Number,
				Bool,
				String,
				Date,
				Class,
				Array,
				Null,
				Custom
			};

			template<JsonParseTypes v>
			using ParseTag = std::integral_constant<JsonParseTypes, v>;

			template<typename JsonMember, typename OutputIterator,
			         typename parse_to_t>
			static constexpr OutputIterator to_string( ParseTag<JsonParseTypes::Bool>,
			                                           OutputIterator it,
			                                           parse_to_t const &value ) {
				static_assert(
				  std::is_same_v<typename JsonMember::parse_to_t, parse_to_t> );
				if( value ) {
					return copy_to_iterator( "true", it );
				}
				return copy_to_iterator( "false", it );
			}

			template<typename JsonMember, typename OutputIterator, typename Optional>
			static constexpr OutputIterator to_string( ParseTag<JsonParseTypes::Null>,
			                                           OutputIterator it,
			                                           Optional const &value ) {
				static_assert( is_valid_optional_v<Optional> );
				if( !value ) {
					it = copy_to_iterator( "null", it );
					return it;
				}
				return to_string<typename JsonMember::sub_type>(
				  ParseTag<JsonMember::sub_type::expected_type>{}, it, *value );
			}

			template<typename JsonMember, typename OutputIterator,
			         typename parse_to_t>
			static constexpr OutputIterator
			to_string( ParseTag<JsonParseTypes::Number>, OutputIterator it,
			           parse_to_t const &value ) {

				static_assert(
				  std::is_same_v<typename JsonMember::parse_to_t, parse_to_t> );

				using ::daw::json::to_strings::to_string;
				using std::to_string;
				return copy_to_iterator( to_string( value ), it );
			}

			template<typename JsonMember, typename OutputIterator,
			         typename parse_to_t>
			static constexpr OutputIterator
			to_string( ParseTag<JsonParseTypes::String>, OutputIterator it,
			           parse_to_t const &value ) {

				static_assert(
				  std::is_same_v<typename JsonMember::parse_to_t, parse_to_t> );

				*it++ = '"';
				it = copy_to_iterator( value, it );
				*it++ = '"';
				return it;
			}

			template<typename JsonMember, typename OutputIterator,
			         typename parse_to_t>
			static constexpr OutputIterator to_string( ParseTag<JsonParseTypes::Date>,
			                                           OutputIterator it,
			                                           parse_to_t const &value ) {
				static_assert(
				  std::is_same_v<typename JsonMember::parse_to_t, parse_to_t> );

				using ::daw::json::is_null;
				if( is_null( value ) ) {
					it = copy_to_iterator( "null", it );
				} else {
					*it++ = '"';
					using namespace ::daw::date_formatting::formats;
					it = copy_to_iterator( ::daw::date_formatting::fmt_string(
					                         "{0}T{1}:{2}:{3}Z", value, YearMonthDay{},
					                         Hour{}, Minute{}, Second{} ),
					                       it );
					*it++ = '"';
				}
				return it;
			}

			template<typename JsonMember, typename OutputIterator,
			         typename parse_to_t>
			static constexpr OutputIterator
			to_string( ParseTag<JsonParseTypes::Class>, OutputIterator it,
			           parse_to_t const &value ) {

				static_assert(
				  std::is_same_v<typename JsonMember::parse_to_t, parse_to_t> );

				return json_parser_description_t<parse_to_t>::template serialize(
				  it, to_json_data( value ) );
			}

			template<typename JsonMember, typename OutputIterator,
			         typename parse_to_t>
			static constexpr OutputIterator
			to_string( ParseTag<JsonParseTypes::Custom>, OutputIterator it,
			           parse_to_t const &value ) {

				static_assert(
				  std::is_same_v<typename JsonMember::parse_to_t, parse_to_t> );

				return copy_to_iterator( typename JsonMember::to_converter_t{}( value ),
				                         it );
			}

			template<typename JsonMember, typename OutputIterator,
			         typename parse_to_t>
			static constexpr OutputIterator
			to_string( ParseTag<JsonParseTypes::Array>, OutputIterator it,
			           parse_to_t const &container ) {

				static_assert(
				  std::is_same_v<typename JsonMember::parse_to_t, parse_to_t> );

				*it++ = '[';
				if( !std::empty( container ) ) {
					auto count = std::size( container ) - 1;
					for( auto const &v : container ) {
						it = to_string<typename JsonMember::json_element_t>(
						  ParseTag<JsonMember::json_element_t::expected_type>{}, it, v );
						if( count-- > 0 ) {
							*it++ = ',';
						}
					}
				}
				*it++ = ']';
				return it;
			}

			template<typename JsonMember, typename OutputIterator, typename T>
			constexpr OutputIterator member_to_string( OutputIterator &&it,
			                                           T &&value ) {
				it = impl::to_string<JsonMember>(
				  impl::ParseTag<JsonMember::expected_type>{},
				  std::forward<OutputIterator>( it ), std::forward<T>( value ) );
				return std::move( it );
			}

			template<typename JsonMember, size_t pos, typename OutputIterator,
			         typename... Args>
			void to_json_str( OutputIterator it, std::tuple<Args...> &&args ) {

				static_assert( is_a_json_type_v<JsonMember>, "Unsupported data type" );
				*it++ = '"';
				it = copy_to_iterator( JsonMember::name, it );
				it = copy_to_iterator( "\":", it );
				it = member_to_string<JsonMember>( std::move( it ),
				                                   std::get<pos>( std::move( args ) ) );
				if constexpr( pos < ( sizeof...( Args ) - 1U ) ) {
					*it++ = ',';
				}
			}

			constexpr char to_lower( char c ) noexcept {
				return static_cast<char>( static_cast<unsigned>( c ) |
				                          static_cast<unsigned>( ' ' ) );
			}

			template<typename Result>
			constexpr Result parse_integer( daw::string_view sv ) noexcept {
				if( sv.empty( ) ) {
					return static_cast<Result>( 0 );
				}
				if constexpr( std::is_signed_v<Result> ) {
					return daw::parser::parse_int<Result>( sv );
				} else {
					return daw::parser::parse_unsigned_int<Result>( sv );
				}
			}

			template<typename Result>
			constexpr Result parse_real( daw::string_view sv ) noexcept {
				// [-]WHOLE[.FRACTION][(e|E)EXPONENT]

				auto const whole_part =
				  static_cast<double>( daw::parser::parse_int<int64_t>(
				    sv.pop_front( sv.find_first_of( ".eE" ) ) ) );

				double fract_part = 0;
				if( !sv.empty( ) and sv.front( ) == '.' ) {
					sv.remove_prefix( );
					auto fract_str = sv.pop_front( sv.find_first_of( "eE" ) );
					auto const fract_exp = static_cast<int32_t>( fract_str.size( ) );
					fract_part = static_cast<double>(
					  daw::parser::parse_unsigned_int<uint64_t>( fract_str ) );
					fract_part *= daw::cxmath::dpow10( -fract_exp );
					fract_part = daw::cxmath::copy_sign( fract_part, whole_part );
				}

				int32_t exp_part = 0;
				if( !sv.empty( ) ) {
					sv.remove_prefix( );
					exp_part = daw::parser::parse_int<int32_t>( sv );
				}
				return static_cast<Result>( ( whole_part + fract_part ) *
				                            daw::cxmath::dpow10( exp_part ) );
			} // namespace

			template<typename T>
			inline constexpr bool has_json_parser_description_v =
			  daw::is_detected_v<json_parser_description_t, T>;

			template<typename T>
			using json_parser_to_json_data_t =
			  decltype( to_json_data( std::declval<T &>( ) ) );

			template<typename T>
			inline constexpr bool has_json_to_json_data_v =
			  daw::is_detected_v<json_parser_to_json_data_t, T>;

			template<typename string_t>
			struct kv_t {
				string_t name;
				JsonParseTypes expected_type;
				// bool nullable;
				size_t pos;

				constexpr kv_t( string_t Name,
				                JsonParseTypes Expected, /*bool Nullable,*/
				                size_t Pos )
				  : name( std::move( Name ) )
				  , expected_type( Expected )
				  //				  , nullable( Nullable )
				  , pos( Pos ) {}
			};

			struct value_pos {
				daw::string_view value_str{};

				constexpr value_pos( ) noexcept = default;
				explicit constexpr value_pos( daw::string_view sv ) noexcept
				  : value_str( sv ) {}
				constexpr bool empty( ) const noexcept {
					return value_str.empty( );
				}

				constexpr decltype( auto ) front( ) const noexcept {
					return value_str.front( );
				}

				constexpr bool at_start_of_array( ) const noexcept {
					return !empty( ) and front( ) == '[';
				}

				constexpr bool at_end_of_array( ) const noexcept {
					return empty( ) or front( ) == ']';
				}

				constexpr void trim_left( ) noexcept {
					value_str = daw::parser::trim_left( value_str );
				}

				constexpr void remove_prefix( size_t count = 1 ) noexcept {
					value_str.remove_prefix( count );
				}
			};

			template<typename JsonType>
			using json_parse_to = typename JsonType::parse_to_t;

			template<typename JsonType>
			inline constexpr bool is_json_nullable_v = JsonType::nullable;

			template<typename JsonType>
			inline constexpr bool is_json_empty_null_v = JsonType::empty_is_null;

			struct member_name_parse_error {};
			constexpr daw::string_view parse_name( daw::string_view &sv ) {
				daw::exception::precondition_check( sv.front( ) == '"' );
				sv.remove_prefix( );
				auto name = sv.pop_front( "\"" );
				sv.pop_front( ":" );
				sv = parser::trim_left( sv );
				return name;
			}

			constexpr daw::string_view skip_string( daw::string_view &sv ) {
				auto result = daw::string_view{};
				if( sv.empty( ) ) {
					return {};
				}
				if( sv.front( ) == '"' ) {
					sv.remove_prefix( );
				}
				auto pos = sv.find_first_of( "\"" );
				exception::precondition_check( pos != daw::string_view::npos,
				                               "Invalid class" );
				while( pos != daw::string_view::npos ) {
					if( pos == 0 or
					    ( pos != daw::string_view::npos and sv[pos - 1] != '\\' ) ) {
						result = sv.pop_front( pos );
						break;
					}
					++pos;
					pos = sv.find_first_of( "\"", pos );
				}
				exception::precondition_check( pos != daw::string_view::npos,
				                               "Invalid class" );
				pos = sv.find_first_of( ",}]\n" );
				exception::precondition_check( pos != daw::string_view::npos,
				                               "Invalid class" );
				sv.remove_prefix( pos + 1 );
				sv = parser::trim_left( sv );
				return result;
			}

			constexpr daw::string_view skip_other( daw::string_view &sv ) {
				auto pos = sv.find_first_of( ",}]\n" );
				exception::precondition_check( pos != daw::string_view::npos,
				                               "Invalid class" );
				auto result = sv.pop_front( pos );
				sv.remove_prefix( );
				sv = parser::trim_left( sv );
				return result;
			}

			template<char Left, char Right>
			constexpr daw::string_view skip_bracketed_item( daw::string_view &sv ) {
				size_t bracket_count = 1;
				bool is_escaped = false;
				bool in_quotes = false;
				auto tmp_sv = sv;
				sv.remove_prefix( );
				while( !sv.empty( ) and bracket_count > 0 ) {
					switch( sv.front( ) ) {
					case '\\':
						if( !in_quotes and !is_escaped ) {
							is_escaped = true;
							sv.remove_prefix( );
							continue;
						}
						break;
					case '"':
						if( !is_escaped ) {
							in_quotes = !in_quotes;
							sv.remove_prefix( );
							continue;
						}
						break;
					case Left:
						if( !in_quotes and !is_escaped ) {
							++bracket_count;
						}
						break;
					case Right:
						if( !in_quotes and !is_escaped ) {
							--bracket_count;
						}
					}
					is_escaped = false;
					sv.remove_prefix( );
				}
				tmp_sv = tmp_sv.pop_front( tmp_sv.size( ) - sv.size( ) );
				auto pos = sv.find_first_of( ",}]\n" );
				struct bracketed_item_parse_exception {};
				exception::precondition_check<bracketed_item_parse_exception>(
				  pos != sv.npos );
				sv.remove_prefix( );
				sv = parser::trim_left( sv );
				return tmp_sv;
			}

			constexpr daw::string_view skip_class( daw::string_view &sv ) {
				return skip_bracketed_item<'{', '}'>( sv );
			}

			constexpr daw::string_view skip_array( daw::string_view &sv ) {
				return skip_bracketed_item<'[', ']'>( sv );
			}

			struct skip_value_result_t {
				daw::string_view sv{};

				bool is_null = false;

				constexpr skip_value_result_t( daw::string_view s ) noexcept
				  : sv( s ) {}
			};

			constexpr skip_value_result_t skip_value( daw::string_view &sv ) {
				daw::exception::precondition_check( !sv.empty( ) );
				switch( sv.front( ) ) {
				case '"':
					return skip_string( sv );
				case '[':
					return skip_array( sv );
				case '{':
					return skip_class( sv );
				default: { return skip_other( sv ); }
				}
			}

			template<JsonParseTypes>
			struct missing_nonnullable_value_expection {};

			template<typename JsonMember>
			constexpr auto parse_value( ParseTag<JsonParseTypes::Number>,
			                            value_pos pos ) {
				// assert !pos.value_str.empty( );
				using constructor_t = typename JsonMember::constructor_t;
				using element_t = typename JsonMember::parse_to_t;

				if constexpr( std::is_floating_point_v<element_t> ) {
					return constructor_t{}( parse_real<element_t>( pos.value_str ) );
				}
				return constructor_t{}( parse_integer<element_t>( pos.value_str ) );
			}

			template<typename JsonMember>
			constexpr auto parse_value( ParseTag<JsonParseTypes::Null>,
			                            value_pos pos ) {
				// assert !pos.value_str.empty( );
				using constructor_t = typename JsonMember::constructor_t;
				using element_t = typename JsonMember::sub_type;

				if( pos.empty( ) or pos.front( ) == 'n' ) {
					return constructor_t{}( );
				}
				return parse_value<element_t>( ParseTag<element_t::expected_type>{},
				                               std::move( pos ) );
			}

			template<typename JsonMember>
			constexpr auto parse_value( ParseTag<JsonParseTypes::Custom>,
			                            value_pos pos ) {
				return
				  typename JsonMember::from_converter_t{}( to_ssv( pos.value_str ) );
			}

			template<typename JsonMember>
			constexpr auto parse_value( ParseTag<JsonParseTypes::Bool>,
			                            value_pos pos ) {
				using constructor_t = typename JsonMember::constructor_t;
				return constructor_t{}( !pos.empty( ) and
				                        to_lower( pos.front( ) ) == 't' );
			}

			template<typename JsonMember>
			constexpr auto parse_value( ParseTag<JsonParseTypes::String>,
			                            value_pos pos ) {

				using constructor_t = typename JsonMember::constructor_t;

				if( JsonMember::empty_is_null and pos.empty( ) ) {
					return constructor_t{}( );
				}
				return constructor_t{}( pos.value_str.data( ), pos.value_str.size( ) );
			}

			template<typename JsonMember>
			constexpr auto parse_value( ParseTag<JsonParseTypes::Date>,
			                            value_pos pos ) {

				using constructor_t = typename JsonMember::constructor_t;
				return constructor_t{}( pos.value_str.data( ), pos.value_str.size( ) );
			}

			template<typename JsonMember>
			constexpr auto parse_value( ParseTag<JsonParseTypes::Class>,
			                            value_pos pos ) {

				using element_t = typename JsonMember::parse_to_t;
				return from_json<element_t>( to_ssv( pos.value_str ) );
			}

			struct invalid_array {};
			template<typename JsonMember>
			constexpr auto parse_value( ParseTag<JsonParseTypes::Array>,
			                            value_pos pos ) {

				using element_t = typename JsonMember::json_element_t;
				daw::exception::precondition_check<invalid_array>(
				  pos.at_start_of_array( ) );

				pos.remove_prefix( );
				pos.trim_left( );

				auto array_container = typename JsonMember::constructor_t{}( );
				auto container_appender =
				  typename JsonMember::appender_t( array_container );

				while( !pos.at_end_of_array( ) ) {
					auto vp = value_pos( skip_value( pos.value_str ).sv );
					container_appender( parse_value<element_t>(
					  ParseTag<element_t::expected_type>{}, std::move( vp ) ) );
				}
				pos.trim_left( );
				return array_container;
			}

			template<typename Container>
			struct basic_appender {
				daw::back_inserter_iterator<Container> appender;

				constexpr basic_appender( Container &container ) noexcept
				  : appender( container ) {}

				template<typename Value>
				constexpr void operator( )( Value &&value ) {
					*appender = std::forward<Value>( value );
				}
			};

			template<size_t N, typename string_t, typename... JsonMembers>
			static constexpr impl::kv_t<string_t> get_item( ) noexcept {
				using type_t = traits::nth_type<N, JsonMembers...>;
				return {type_t::name, type_t::expected_type, /*type_t::nullable,*/ N};
			}

			template<typename... JsonMembers>
			constexpr size_t find_string_capacity( ) noexcept {
				return ( json_name_len( JsonMembers::name ) + ... );
			}

			template<typename... JsonMembers, size_t... Is>
			constexpr auto make_map( std::index_sequence<Is...> ) noexcept {
				using string_t =
				  basic_bounded_string<char, find_string_capacity<JsonMembers...>( )>;

				return daw::make_array( get_item<Is, string_t, JsonMembers...>( )... );
			}

			template<typename... JsonMembers>
			struct name_map_t {
				static constexpr auto name_map = impl::make_map<JsonMembers...>(
				  std::index_sequence_for<JsonMembers...>{} );

				static constexpr bool has_name( daw::string_view key ) noexcept {
					using std::begin;
					using std::end;
					auto result = algorithm::find_if(
					  begin( name_map ), end( name_map ),
					  [key]( auto const &kv ) { return kv.name == key; } );
					return result != std::end( name_map );
				}

				static constexpr size_t find_name( daw::string_view key ) noexcept {
					using std::begin;
					using std::end;
					auto result = algorithm::find_if(
					  begin( name_map ), end( name_map ),
					  [key]( auto const &kv ) { return kv.name == key; } );
					if( result == std::end( name_map ) ) {
						std::terminate( );
					}
					return static_cast<size_t>(
					  std::distance( begin( name_map ), result ) );
				}
			};
			struct location_info_t {
				JSONNAMETYPE name;
				std::optional<daw::string_view> location{};

				constexpr bool missing( ) const {
					return !static_cast<bool>( location );
				}
			};

			template<size_t JsonMemberPosition, typename... JsonMembers>
			constexpr decltype( auto )
			parse_item( std::array<impl::value_pos, sizeof...( JsonMembers )> const
			              &locations ) {

				using JsonMember = traits::nth_type<JsonMemberPosition, JsonMembers...>;

				return impl::parse_value<JsonMember>(
				  impl::ParseTag<JsonMember::expected_type>{},
				  locations[JsonMemberPosition] );
			}

			constexpr bool at_end_of_class( daw::string_view const sv ) noexcept {
				return sv.empty( ) or sv.front( ) == '}';
			}

			template<size_t pos, typename... JsonMembers>
			constexpr void find_location(
			  std::array<location_info_t, sizeof...( JsonMembers )> &locations,
			  daw::string_view &sv ) {

				using type_t = traits::nth_type<pos, JsonMembers...>;
				size_t idx = pos;
				daw::exception::precondition_check(
				  !locations[idx].missing( ) or
				  ( !sv.empty( ) and sv.front( ) != '}' ) );

				while( locations[idx].missing( ) ) {
					sv = parser::trim_left( sv );
					if( at_end_of_class( sv ) ) {
						break;
					}
					auto name = impl::parse_name( sv );
					auto v = impl::skip_value( sv );
					sv = parser::trim_left( sv );
					if( !name_map_t<JsonMembers...>::has_name( name ) ) {
						continue;
					}
					auto const name_pos = name_map_t<JsonMembers...>::find_name( name );
					if( v.is_null ) {
						if( type_t::expected_type == JsonParseTypes::Null ) {
							return;
						}
						throw impl::missing_nonnullable_value_expection<
						  impl::JsonParseTypes::Null>{};
					}
					locations[name_pos].location = {v.sv};
				}
			}

			template<size_t JsonMemberPosition, typename... JsonMembers>
			constexpr decltype( auto ) parse_item(
			  std::array<location_info_t, sizeof...( JsonMembers )> &locations,
			  daw::string_view &sv ) {

				find_location<JsonMemberPosition, JsonMembers...>( locations, sv );

				using JsonMember = traits::nth_type<JsonMemberPosition, JsonMembers...>;

				auto vp = value_pos{};
				if( locations[JsonMemberPosition].location ) {
					vp.value_str = *locations[JsonMemberPosition].location;
				}
				return impl::parse_value<JsonMember>(
				  impl::ParseTag<JsonMember::expected_type>{}, std::move( vp ) );
			}

			template<size_t N, typename... JsonMembers>
			using nth = daw::traits::nth_element<N, JsonMembers...>;

			template<typename... JsonMembers, typename OutputIterator, size_t... Is,
			         typename... Args>
			constexpr OutputIterator
			serialize_json_class( OutputIterator it, std::index_sequence<Is...>,
			                      std::tuple<Args...> &&args ) {

				*it++ = '{';

				(void)( ( to_json_str<nth<Is, JsonMembers...>, Is>( it,
				                                                    std::move( args ) ),
				          ... ),
				        0 );

				*it++ = '}';
				return it;
			}

			template<typename Result, typename... JsonMembers, size_t... Is>
			constexpr Result parse_json_class( daw::string_view sv,
			                                   std::index_sequence<Is...> ) {
				static_assert(
				  can_construct_a_v<Result, typename JsonMembers::parse_to_t...>,
				  "Supplied types cannot be used for construction of this type" );

				auto known_locations =
				  daw::make_array( impl::location_info_t{JsonMembers::name}... );

				sv = parser::trim_left( sv );
				exception::precondition_check( !sv.empty( ) and sv.front( ) == '{' );
				sv.remove_prefix( );
				sv = parser::trim_left( sv );

				return construct_a<Result>{}(
				  impl::parse_item<Is, JsonMembers...>( known_locations, sv )... );
			}
		} // namespace impl
	}   // namespace json
} // namespace daw
