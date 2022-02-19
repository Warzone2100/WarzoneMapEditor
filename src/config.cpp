#include <libconfig.h++>
#include "config.h"
#include <log.hpp>
#include <src/build.h>

using namespace libconfig;

Config wmecfg;

void ReadGlobalConfig(){
  log_info("Reading config file.");
  try{
    wmecfg.readFile(CURRENT_DIR"/wmeglobal.cfg");
  }
  catch(const FileIOException &fioex){
    log_error("I/O error while reading file.");
  }
  catch(const ParseException &pex)
  {
    log_error("Parse error at %s:%u:%s",pex.getFile(),pex.getLine(),pex.getError());
    }
    log_info("Reading config OK");
    //-----Log level
    int log_level;
    if(wmecfg.lookupValue("WMEditor.Log_level",log_level)){
      log_set_level(log_level);
    }
    else{
      log_error("Unknown debug level.");
    }
    //-----
  }
