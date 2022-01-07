execute_process(COMMAND git log --pretty=format:'%h' -n 1
                OUTPUT_VARIABLE GIT_REV
                ERROR_QUIET)
if ("${GIT_REV}" STREQUAL "")
    set(GIT_REV "N/A")
    set(GIT_DIFF "")
    set(GIT_TAG "0.0")
    set(GIT_BRANCH "N/A")
	message(WARNING "No git version detected!")
else()
    execute_process(
        COMMAND bash -c "git diff --quiet --exit-code || echo +"
        OUTPUT_VARIABLE GIT_DIFF)
    execute_process(
        COMMAND git describe --tags
	OUTPUT_VARIABLE GIT_TAG_RAW ERROR_QUIET)
    execute_process(
        COMMAND git rev-parse --abbrev-ref HEAD
        OUTPUT_VARIABLE GIT_BRANCH)

    string(STRIP "${GIT_REV}" GIT_REV)
    string(SUBSTRING "${GIT_REV}" 1 7 GIT_REV)
    string(STRIP "${GIT_DIFF}" GIT_DIFF)
    string(STRIP "${GIT_TAG_RAW}" GIT_TAG_RAW)
    string(STRIP "${GIT_BRANCH}" GIT_BRANCH)
	string(REGEX MATCH "^[0-9]+\\.[0-9]+\\.[0-9]+" GIT_TAG "${GIT_TAG_RAW}")
	if ("${GIT_TAG}" STREQUAL "")
		set(GIT_TAG "0.0")
		# message(WARNING "Git tag is empty, defaulting to 0.0")
	endif()
	message(STATUS "Git: ${GIT_REV} ${GIT_BRANCH} ${GIT_TAG}")
endif()
