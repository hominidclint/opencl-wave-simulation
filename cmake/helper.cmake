# Common helper methods and macros
# This module defines
# configureSourceGroups(), adds source filters to the project
# configureDebugPostfix(), appends debug binaries with the CMAKE_DEBUG_POSTFIX specified
#
# Created by Hannes Wuerfel

function(configureSourceGroups)
	SOURCE_GROUP(src REGULAR_EXPRESSION "c$|cpp$|ui$|qrc$")
	SOURCE_GROUP(header REGULAR_EXPRESSION "hpp$|h$")
	SOURCE_GROUP(cuda REGULAR_EXPRESSION "cu$")
	SOURCE_GROUP(ocl REGULAR_EXPRESSION "cl$|ocl$")
	SOURCE_GROUP(generated REGULAR_EXPRESSION "cxx$|ui_|ptx")
	SOURCE_GROUP(fx REGULAR_EXPRESSION "fx$|glsl$|hlsl$|geom$|vert$|frag$")
endfunction(configureSourceGroups)

function(configureDebugPostfix suffix)
	set_target_properties(${target} 
		PROPERTIES DEBUG_POSTFIX ${suffix}
		RELEASE_POSTFIX ""
	) 
endfunction()