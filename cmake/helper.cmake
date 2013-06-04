# Common helper methods and macros
# This module defines
# configureSourceGroups(), adds source filters to the project
# configureDebugPostfix(), appends debug binaries with the CMAKE_DEBUG_POSTFIX specified
#
# Created by Hannes Wuerfel

function(configureSourceGroups)
	SOURCE_GROUP(src REGULAR_EXPRESSION "c$|cpp$|hpp$|h$|ui$|qrc$")
	SOURCE_GROUP(ocl REGULAR_EXPRESSION "cl$|ocl$")
	SOURCE_GROUP(fx REGULAR_EXPRESSION "fx$|glsl$|hlsl$|geom$|vert$|frag$")
endfunction(configureSourceGroups)

function(configureDebugPostfix suffix)
	set_target_properties(${target} 
		PROPERTIES DEBUG_POSTFIX ${suffix}
		RELEASE_POSTFIX ""
	) 
endfunction()