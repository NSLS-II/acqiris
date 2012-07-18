TOP=../..
include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE

#----------------------------------------------------
#  Optimization of db files using dbst (DEFAULT: NO)
#DB_OPT = YES

#----------------------------------------------------
# Create and install (or just install)
# databases, templates, substitutions like this
#DB += dbExample1.db
#DB += dbExample2.db
#DB += dbSubExample.db
#DB += user.substitutions
#DB += userHost.substitutions

#Yong Hu:db/template files
#DB += acqiris_channel.template
#DB += acqiris_channel.substitutions
#DB += acqiris_trigger.template
#DB += acqiris_trigger.substitutions
#DB += acqiris_module.template
#DB += acqiris_module.substitutions
DB += acqiris_channel.db
DB += acqiris_trigger.db
DB += acqiris_module.db
#DB += iocStats.db

#DB_INSTALLS += $(EPICS_BASE)/db/iocAdminSoft.db
#DB_INSTALLS += $(EPICS_BASE)/db/save_restoreStatus.db

#----------------------------------------------------
# If <anyname>.db template is not named <anyname>*.template add
# <anyname>_TEMPLATE = <templatename>

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

