// Copyright (c) Darrell Wright
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/beached/daw_json_link
//

#pragma once

#include "daw_json_assert.h"
#include "daw_json_parse_common.h"
#include "daw_parse_policy_policy_details.h"

#include <daw/cpp_17.h>
#include <daw/daw_hide.h>

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <type_traits>

namespace daw::json {
	template<bool IsUncheckedInput, SIMDModes SIMDMode, bool AllowEscapedNames>
	struct BasicNoCommentSkippingPolicy {
		using iterator = char const *;
		static inline constexpr bool is_unchecked_input = IsUncheckedInput;
		static inline constexpr SIMDModes simd_mode = SIMDMode;
		static inline constexpr bool allow_escaped_names = AllowEscapedNames;
		using CharT = char;

		using as_unchecked =
		  BasicNoCommentSkippingPolicy<true, simd_mode, allow_escaped_names>;
		using as_checked =
		  BasicNoCommentSkippingPolicy<false, simd_mode, allow_escaped_names>;

		using policy = BasicNoCommentSkippingPolicy;

		iterator first{};
		iterator last{};
		iterator class_first{};
		iterator class_last{};
		std::size_t counter = 0;
		using Range = BasicNoCommentSkippingPolicy;

		template<std::size_t N>
		inline constexpr bool operator==( char const ( &rhs )[N] ) const {
			if( size( ) < ( N - 1 ) ) {
				return false;
			}
			bool result = true;
			for( std::size_t n = 0; n < ( N - 1 ); ++n ) {
				result = result and ( first[n] == rhs[n] );
			}
			return result;
		}

		inline constexpr BasicNoCommentSkippingPolicy( ) = default;

		inline constexpr BasicNoCommentSkippingPolicy( iterator f, iterator l )
		  : first( f )
		  , last( l )
		  , class_first( f )
		  , class_last( l ) {

			assert( f );
			assert( l );
			daw_json_assert( not is_null( ), "Unexpceted null start of range" );
		}

		[[nodiscard]] DAW_ATTRIBUTE_FLATTEN inline constexpr bool empty( ) const
		  noexcept {
			if constexpr( is_unchecked_input ) {
				return first == last;
			} else {
				return first != nullptr and first == last;
			}
		}

		[[nodiscard]] DAW_ATTRIBUTE_FLATTEN inline constexpr bool has_more( ) const
		  noexcept {
			return first < last;
		}

		DAW_ATTRIBUTE_FLATTEN inline constexpr void
		move_to_next_of_unchecked( char c ) {
			while( *first != c ) {
				++first;
			}
		}

		DAW_ATTRIBUTE_FLATTEN inline constexpr void
		move_to_next_of_checked( char c ) {
			while( has_more( ) and *first != c ) {
				++first;
			}
		}

		template<std::size_t N, std::size_t... Is>
		inline constexpr void
		move_to_next_of_nc_unchecked( char const ( &str )[N],
		                              std::index_sequence<Is...> ) {
			while( ( ( *first != str[Is] ) and ... ) ) {
				++first;
			}
		}

		template<std::size_t N, std::size_t... Is>
		inline constexpr void
		move_to_next_of_nc_checked( char const ( &str )[N],
		                            std::index_sequence<Is...> ) {
			while( has_more( ) and ( ( *first != str[Is] ) and ... ) ) {
				++first;
			}
		}

		template<std::size_t N>
		inline constexpr void move_to_next_of_nc( char const ( &str )[N] ) {
			if constexpr( is_unchecked_input ) {
				move_to_next_of_nc_unchecked( str, std::make_index_sequence<N>{} );
			} else {
				move_to_next_of_nc_checked( str, std::make_index_sequence<N>{} );
			}
		}

		[[nodiscard]] inline constexpr decltype( auto ) front( ) const noexcept {
			return *first;
		}

		[[nodiscard]] inline constexpr bool front( char c ) const noexcept {
			return first < last and *first == c;
		}

		[[nodiscard]] inline constexpr std::size_t size( ) const noexcept {
			return static_cast<std::size_t>( std::distance( first, last ) );
		}

		[[nodiscard]] inline constexpr bool is_number( ) const noexcept {
			return static_cast<unsigned>( front( ) ) - static_cast<unsigned>( '0' ) <
			       10U;
		}

		template<std::size_t N>
		[[nodiscard]] inline constexpr bool front( char const ( &set )[N] ) const {
			assert( first );
			if( empty( ) ) {
				return false;
			}
			bool result = false;
			for( std::size_t n = 0; n < ( N - 1 ); ++n ) {
				result |= parse_policy_details::in( *first, set[n] );
			}
			return result;
		}

		[[nodiscard]] inline constexpr bool is_null( ) const noexcept {
			return first == nullptr;
		}

		inline constexpr void remove_prefix( ) {
			++first;
		}

		inline constexpr void remove_prefix( std::size_t n ) noexcept {
			first += static_cast<intmax_t>( n );
		}

		[[nodiscard]] inline constexpr iterator begin( ) const noexcept {
			return first;
		}

		[[nodiscard]] inline constexpr iterator data( ) const noexcept {
			return first;
		}

		[[nodiscard]] inline constexpr iterator end( ) const noexcept {
			return last;
		}

		[[nodiscard]] explicit inline constexpr operator bool( ) const noexcept {
			return not empty( );
		}

		inline constexpr void set_class_position( ) {
			class_first = first;
			class_last = last;
		}

		inline constexpr bool at_literal_end( ) const noexcept {
			char const c = *first;
			return c == '\0' or c == ',' or c == ']' or c == '}';
		}

		inline constexpr bool is_space( ) const noexcept( is_unchecked_input ) {
			daw_json_assert_weak( has_more( ), "Unexpected end" );
			return *first <= 0x20;
		}

		inline constexpr bool is_space_unchecked( ) const noexcept {
			return *first <= 0x20;
		}

		inline constexpr void trim_left_checked( ) noexcept {
			while( has_more( ) and is_space_unchecked( ) ) {
				++first;
			}
		}

		inline constexpr void trim_left_unchecked( ) noexcept {
			while( is_space_unchecked( ) ) {
				++first;
			}
		}

		inline constexpr void trim_left( ) noexcept( is_unchecked_input ) {
			if constexpr( is_unchecked_input ) {
				trim_left_unchecked( );
			} else {
				trim_left_checked( );
			}
		}

		inline constexpr void clean_tail_unchecked( ) noexcept {
			trim_left_unchecked( );
			if( *first == ',' ) {
				++first;
				trim_left_unchecked( );
			}
		}

		inline constexpr void clean_tail_checked( ) {
			trim_left_checked( );
			if( front( ',' ) ) {
				++first;
				trim_left_checked( );
			}
		}

		inline constexpr void clean_tail( ) noexcept( is_unchecked_input ) {
			if constexpr( is_unchecked_input ) {
				clean_tail_unchecked( );
			} else {
				clean_tail_unchecked( );
			}
		}

		inline constexpr void
		move_to_next_of( char c ) noexcept( is_unchecked_input ) {
			daw_json_assert_weak( has_more( ), "Unexpected end of data" );
			while( *first != c ) {
				++first;
				daw_json_assert_weak( has_more( ), "Unexpected end of data" );
			}
		}

		template<std::size_t N>
		inline constexpr void
		move_to_next_of( char const ( &str )[N] ) noexcept( is_unchecked_input ) {
			daw_json_assert_weak( has_more( ), "Unexpected end of data" );
			while( not parse_policy_details::in( *first, str ) ) {
				++first;
				daw_json_assert_weak( has_more( ), "Unexpected end of data" );
			}
		}

		template<char PrimLeft, char PrimRight, char SecLeft, char SecRight>
		[[nodiscard]] DAW_ATTRIBUTE_FLATTEN inline constexpr Range
		skip_bracketed_item_checked( ) noexcept( is_unchecked_input ) {
			// Not checking for Left as it is required to be skipped already
			auto result = *this;
			std::size_t cnt = 0;
			std::uint32_t prime_bracket_count = 1;
			std::uint32_t second_bracket_count = 0;
			char const *ptr_first = first;
			char const *const ptr_last = last;
			if( ptr_first < ptr_last and *ptr_first == PrimLeft ) {
				++ptr_first;
			}
			while( ptr_first < ptr_last and prime_bracket_count > 0 ) {
				switch( *ptr_first ) {
				case '\\':
					++ptr_first;
					break;
				case '"':
					++ptr_first;
					while( ptr_first < ptr_last and *ptr_first != '"' ) {
						if( *ptr_first == '\\' ) {
							++ptr_first;
							if( ptr_first >= ptr_last ) {
								break;
							}
						}
						++ptr_first;
					}
					daw_json_assert( ptr_first < ptr_last, "Unexpected end of stream" );
					break;
				case ',':
					if( prime_bracket_count == 1 and second_bracket_count == 0 ) {
						++cnt;
					}
					break;
				case PrimLeft:
					++prime_bracket_count;
					break;
				case PrimRight:
					--prime_bracket_count;
					break;
				case SecLeft:
					++second_bracket_count;
					break;
				case SecRight:
					--second_bracket_count;
					break;
				}
				++ptr_first;
			}
			daw_json_assert_weak( prime_bracket_count == 0 and
			                        second_bracket_count == 0,
			                      "Unexpected bracketing" );
			// We include the close primary bracket in the range so that subsequent
			// parsers have a terminator inside their range
			result.last = ptr_first;
			result.counter = cnt;
			first = ptr_first;
			return result;
		}

		template<char PrimLeft, char PrimRight, char SecLeft, char SecRight>
		[[nodiscard]] DAW_ATTRIBUTE_FLATTEN inline constexpr Range
		skip_bracketed_item_unchecked( ) noexcept( is_unchecked_input ) {
			// Not checking for Left as it is required to be skipped already
			auto result = *this;
			std::size_t cnt = 0;
			std::uint32_t prime_bracket_count = 1;
			std::uint32_t second_bracket_count = 0;
			char const *ptr_first = first;
			if( *ptr_first == PrimLeft ) {
				++ptr_first;
			}
			while( prime_bracket_count > 0 ) {
				switch( *ptr_first ) {
				case '\\':
					++ptr_first;
					break;
				case '"':
					++ptr_first;
					while( *ptr_first != '"' ) {
						if( *ptr_first == '\\' ) {
							++ptr_first;
						}
						++ptr_first;
					}
					break;
				case ',':
					if( prime_bracket_count == 1 and second_bracket_count == 0 ) {
						++cnt;
					}
					break;
				case PrimLeft:
					++prime_bracket_count;
					break;
				case PrimRight:
					--prime_bracket_count;
					break;
				case SecLeft:
					++second_bracket_count;
					break;
				case SecRight:
					--second_bracket_count;
					break;
				}
				++ptr_first;
			}
			// We include the close primary bracket in the range so that subsequent
			// parsers have a terminator inside their range
			result.last = ptr_first;
			result.counter = cnt;
			first = ptr_first;
			return result;
		}

		[[nodiscard]] inline constexpr Range
		skip_class( ) noexcept( is_unchecked_input ) {
			if constexpr( is_unchecked_input ) {
				return skip_bracketed_item_unchecked<'{', '}', '[', ']'>( );
			} else {
				return skip_bracketed_item_checked<'{', '}', '[', ']'>( );
			}
		}

		[[nodiscard]] inline constexpr Range
		skip_array( ) noexcept( is_unchecked_input ) {
			if constexpr( is_unchecked_input ) {
				return skip_bracketed_item_unchecked<'[', ']', '{', '}'>( );
			} else {
				return skip_bracketed_item_checked<'[', ']', '{', '}'>( );
			}
		}
	};

	using NoCommentSkippingPolicyChecked =
	  BasicNoCommentSkippingPolicy<false, SIMDModes::None, false>;

	using NoCommentSkippingPolicyUnchecked =
	  BasicNoCommentSkippingPolicy<true, SIMDModes::None, false>;

	template<SIMDModes mode>
	using SIMDNoCommentSkippingPolicyChecked =
	  BasicNoCommentSkippingPolicy<false, mode, false>;

	template<SIMDModes mode>
	using SIMDNoCommentSkippingPolicyUnchecked =
	  BasicNoCommentSkippingPolicy<true, mode, false>;

} // namespace daw::json
