// Copyright (c) Darrell Wright
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/beached/
//

#include "geojson_json.h"

#include <daw/json/daw_from_json.h>

#include <string_view>

namespace DAW_JSON_NS {
	template daw::geojson::Polygon
	from_json<daw::geojson::Polygon,
	          SIMDNoCommentSkippingPolicyUnchecked<simd_exec_tag>>(
	  std::string_view json_data, std::string_view path );

	template daw::geojson::Polygon
	from_json<daw::geojson::Polygon,
	          SIMDNoCommentSkippingPolicyUnchecked<simd_exec_tag>>(
	  std::string_view json_data );
} // namespace DAW_JSON_NS
