idf_component_register(
    SRCS "client.c"
        "list.c"
        "protocol.c"
        "request.c"
        "response.c"
        "server.c"
    INCLUDE_DIRS "."
    REQUIRES buffer
)
