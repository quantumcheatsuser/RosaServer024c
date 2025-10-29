cmake_minimum_required (VERSION 3.8)

add_compile_definitions (
	SOL_ALL_SAFETIES_ON
)

project ("RosaServer")

# Include sub-projects.
add_subdirectory ("RosaServer")
add_subdirectory ("RosaServerSatellite")
