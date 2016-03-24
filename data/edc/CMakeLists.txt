ADD_CUSTOM_TARGET(myaccount.edj
		COMMAND edje_cc -id ${CMAKE_CURRENT_SOURCE_DIR}/data/edc
		${CMAKE_SOURCE_DIR}/data/edc/myaccount.edc ${CMAKE_BINARY_DIR}/data/edc/myaccount.edj
		DEPENDS ${CMAKE_SOURCE_DIR}/data/edc/myaccount.edc
)

ADD_CUSTOM_TARGET(myaccount_edc_images.edj
		COMMAND edje_cc -id ${CMAKE_CURRENT_SOURCE_DIR}/data/edc
		${CMAKE_SOURCE_DIR}/data/edc/myaccount_edc_images.edc ${CMAKE_BINARY_DIR}/data/edc/myaccount_edc_images.edj
		DEPENDS ${CMAKE_SOURCE_DIR}/data/edc/myaccount_edc_images.edc
)

ADD_CUSTOM_TARGET(myaccount_nocontent.edj
		COMMAND edje_cc -id ${CMAKE_CURRENT_SOURCE_DIR}/data/edc
		${CMAKE_SOURCE_DIR}/data/edc/myaccount_edc_images.edc ${CMAKE_BINARY_DIR}/data/edc/myaccount_nocontent.edj
		DEPENDS ${CMAKE_SOURCE_DIR}/data/edc/myaccount_nocontent.edc
)

INSTALL(FILES ${CMAKE_BINARY_DIR}/data/edc/myaccount.edj DESTINATION share/edje/myaccount)
INSTALL(FILES ${CMAKE_BINARY_DIR}/data/edc/myaccount_edc_images.edj DESTINATION share/edje/myaccount)
INSTALL(FILES ${CMAKE_BINARY_DIR}/data/edc/myaccount_nocontent.edj DESTINATION share/edje/myaccount)

