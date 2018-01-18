#if defined(USE_MODULE_MAPITOOLS) && (USE_MODULE_MAPITOOLS == USE_MODULE_AS_PRECOMPILED_SOURCES OR USE_MODULE_MAPITOOLS == USE_MODULE_FULL_SOURCES)
#	include "MapiToolsModule.cpp"
#	include "MessageStore.cpp"
#	include "MapiSession.cpp"
#	include "ExtendedRuleActions.cpp"
#	include "ExtendedRuleCondition.cpp"
#	include "ExchangeModifyTable.cpp"
#	include "MapiFolder.cpp"
#	include "MapiTable.cpp"
#endif
