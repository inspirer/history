// jmail.cpp

#include "stdafx.h"

const char version[] = "1-009";
const char jmail[] = "jMail";
const char build[] = "/build " __DATE__;
const char version_mod[] = ".NT";
static char cname[128] = "jMail.ini";

__declspec(thread) context *g;

int main(int argc, char* argv[])
{
  try { 
    printf("%s %s%s%s\n",jmail,version,version_mod,build);

    g=new context;
    g->line_name="Master";
    g->color_name="colors";
    g->z.readconfig(cname,g->color_name);
    g->v.readconfig(cname,g->line_name);

    g->log->initlog();
    g->mdm->init(g->v.comport,g->v.baudrate);
    g->f->cn->initconsole();
    g->f->init(g->line_name);
    
    g->circle();

    g->f->deinit();
    g->f->cn->deinitconsole();
    g->mdm->deinit();
    g->log->deinitlog();
    delete g;
  }
  catch(errors& e){
    cout << "jmail: fatal: " << e.err << endl;
    exit(0);
  }

  return 0;
}
