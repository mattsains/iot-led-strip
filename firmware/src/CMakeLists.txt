set(app_sources
	"main.c"
	"wifi.c"
	"led.c"
)

idf_component_register(SRCS ${app_sources})
