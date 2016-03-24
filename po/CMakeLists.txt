# for i18n

SET(POFILES
ar.po
da.po
es_ES.po
fr.po
hy.po
ko_KR.po
pl.po
sl.po
zh_CN.po
az.po
de.po
es_US.po
ga.po
is.po
lt.po
pt_BR.po
sr.po
zh_HK.po
bg.po
el_GR.po
et.po
gl.po
it_IT.po
lv.po
pt_PT.po
sv.po
zh_TW.po
ca.po
en_PH.po
eu.po
hi.po
ja_JP.po
mk.po
ro.po
tr_TR.po
en.po
fi.po
hr.po
ka.po
nb.po
ru_RU.po
uk.po
cs.po
en_US.po
fr_CA.po
hu.po
kk.po
nl.po
sk.po
uz.po
)

SET(MSGFMT "/usr/bin/msgfmt")

FOREACH(pofile ${POFILES})
	SET(pofile ${CMAKE_CURRENT_SOURCE_DIR}/${pofile})
	MESSAGE("PO: ${pofile}")
	GET_FILENAME_COMPONENT(absPofile ${pofile} ABSOLUTE)
	GET_FILENAME_COMPONENT(lang ${absPofile} NAME_WE)
	SET(moFile ${CMAKE_CURRENT_BINARY_DIR}/${lang}.mo)
	ADD_CUSTOM_COMMAND(
			OUTPUT ${moFile}
			COMMAND ${MSGFMT} -o ${moFile} ${absPofile}
			DEPENDS ${absPofile}
	)
	INSTALL(FILES ${moFile}
			DESTINATION ${RESDIR}/locale/${lang}/LC_MESSAGES RENAME ${PROJECT_NAME}.mo)
	SET(moFiles ${moFiles} ${moFile})
ENDFOREACH(pofile)

MESSAGE(".mo files: ${moFiles}")
ADD_CUSTOM_TARGET(po ALL DEPENDS ${moFiles})
