// defs.c

//#define _DEBUG_asm
//#define _DEBUG_process

#include "i386.h"
#include "cc.h"

char idname[256],*idn,*idn2;

int _deep,_cnter,_up_down[max_deep];

void asm_code() // line = "asmcmd;..."
{
  char *_ln,*_lt,*p,*o,*so;
  char nlast,ppar,t[128];
  long a,b,c,d;

  _ln=_line=line;_lt=_lntp=lntp;
  BuildImage(0);if (*_line!=0) goto _start_asm_code;
  while(*_line!=0){
    _line++;_lntp++; _start_asm_code:
    if (*_lntp=='.'){ *_line='+';*_lntp='+';}
    if(*_lntp==';'||*_lntp==0||*_lntp=='{'||*_lntp=='}'){
      nlast=*_lntp;
      *_lntp=0;*_line=0;*t=0;

      #ifdef _DEBUG_asm
        fprintf(stderr,"#code>%s<\n",_ln);
      #endif

      next_prefix:
      while(*_ln==' '){ _ln++;_lt++;}
      a=strlen(_ln);a--;while(*_ln&&_lt[a]==' '){ _ln[a]=0;_lt[a]=0;a--;}

      if(*_lt=='i'){
        p=_ln;while(*_lt=='i'){ _ln++;_lt++;}
        if(*_ln==' '||*_ln==0||*_ln==':'){
          ppar=*_ln;
          if (ppar){
           *_ln=0;*_lt=0;
           _ln++;_lt++;
          }
          if (ppar==':'){
            if (*idname!=0) strcat(idname,",");
            strcat(idname,p);
            goto next_prefix;
          }
          if (i386prefix(strlwr(p))){
            if (*t!=0) strcat(t,",");
            strcat(t,p);
            goto next_prefix;
          }

          // create structure at pi
          if (pi!=NULL){
            pi->next=(void *)malloc(sizeof(*pi));
            if (pi->next==NULL) error("memory allocation #1",efatal);
            pi=pi->next;
          } else {
            pi=(void *)malloc(sizeof(*pi));ins=pi;
            if (pi==NULL) error("memory allocation #2",efatal);
          }

          // save names assigned to this code
          idn=idn2=idname;
          if (*idn){
            while(*idn!=0){
              if (*idn==','){
                *idn=0;AddIdent(idn2,pi);idn2=idn;idn2++;
              } idn++;
            } AddIdent(idn2,pi);
            idname[0]=0;
          }

          // Get params + save in structure
          pi->params[0]=pi->params[1]=pi->params[2]=NULL;
          pi->ptype[0]=pi->ptype[1]=pi->ptype[2]=0;pi->direction=0;
          while(*_ln==' '){ _ln++;_lt++;}
          if (ppar){
            pi->params[0]=(char *)malloc(strlen(_ln)+1);
            if (pi->params[0]==NULL) error("memory allocation #3",efatal);

            // fix jumps in params : up(00)
            while ((o=strstr(_ln,"up("))!=NULL){
              o+=3;if (*o>='0'&&*o<='9'&&*(o+1)>='0'&&*(o+1)<='9'&&*(o+2)==')'){
                a=(*o-'0')*10+(*(o+1)-'0');o--;*o++='_';
                if (a>=_deep) error("wrong up() parameter",eerror);
                else {
                  a=_deep-a;a--;a=_up_down[a];
                  for (b=0;b<3;b++){ *o++='a'+a%26;a=a/26;}
                }
              } else {
                o--;*o++='_';error("up() syntax error",eerror);
              }
            }

            // fix jumps in params : down(00)
            while ((o=strstr(_ln,"down("))!=NULL){
              o+=5;if (*o>='0'&&*o<='9'&&*(o+1)>='0'&&*(o+1)<='9'&&*(o+2)==')'){
                a=(*o-'0')*10+(*(o+1)-'0');o--;*o++='_';
                if (a>=_deep) error("wrong down() parameter",eerror);
                else {
                  a=_deep-a;a--;a=_up_down[a];
                  for (b=0;b<3;b++){ *o++='a'+a%26;a=a/26;}
                }
              } else {
                o--;*o++='_';error("down() syntax error",eerror);
              }
            }

            // save params
            strcpy(pi->params[0],_ln);

            #ifdef _DEBUG_asm
              fprintf(stderr,"#allparams:%s;\n",pi->params[0]);
            #endif

            // split params
            if (strcmp(p,"db")!=0&&strcmp(p,"dw")!=0&&strcmp(p,"dd")!=0&&strcmp(p,"org")!=0&&strcmp(p,"xcode")!=0){
              o=pi->params[0];a=0;
              while(a<2&&*o!=0){
                if (*_lt==','){

                  // save + Kill spaces
                  a++;*o=0;so=o;so++;while(*so==' ') so++;
                  pi->params[a]=so;d=strlen(so);d--;while(*so&&so[d]==' ') so[d--]=0;
                }
                _lt++;o++;
              }
              so=pi->params[0];d=strlen(so);d--;while(*so&&so[d]==' ') so[d--]=0;
              a++;pi->pnum=a;
              #ifdef _DEBUG_asm
                for(a=0;a<pi->pnum;a++)fprintf(stderr,"#param%i:%s;\n",a,pi->params[a]);
              #endif
            } else pi->pnum=-1;

          } else pi->pnum=0;

          // param type detecting
          for (a=0;a<pi->pnum;a++){

            // Register
            for (b=0;*(i386_regtab[b].reg_name)!=0;b++)
               if (!strcmp(i386_regtab[b].reg_name,pi->params[a]))
                    pi->ptype[a]=i386_regtab[b].reg_type;

            // Memory
            b=strlen(pi->params[a]);
            if (pi->ptype[a]==0&&pi->params[a][--b]==']'){

               // Prefixes
               c=0;o=pi->params[a];
               while (strchr(o,':')!=NULL){
                 while(*o!=':') o++;
                 *o=0;o++;strlwr(pi->params[a]);

                 // null prefix
                 if (*(pi->params[a])==0) error("null prefix encountered",ewarn);

                 // data type
                 else if (pi->params[a][1]==0) switch(*(pi->params[a])){
                   case 'b': c=Mem8;break;
                   case 'w': c=Mem16;break;
                   case 'd': c=Mem32;break;
                   case 'q': c=MemFL;break;
                   default : error("unknown data type (prefix error)",eerror);

                 // segment prefix
                 } else if(i386prefix(pi->params[a])){

                    if (*t!=0) strcat(t,",");strcat(t,pi->params[a]);

                 // unknown prefix
                 } else error("unknown prefix",eerror);

                 pi->params[a]=o;
               }

               if (*(pi->params[a])!='[') error("not memory construction",eerror);
               else if (c==0) pi->ptype[a]=Mem;   // Mem
                         else pi->ptype[a]=c;     // Mem8/16/32
            }

            // Imm/Disp
            if (pi->ptype[a]==0)
              pi->ptype[a]=Imm|(Disp8|Disp16|Disp32|DispXX);

          }

          // init structure
          if (*t==0) pi->prefix=NULL;
                else pi->prefix=(char *)malloc(strlen(t)+1);
          pi->command=(char *)malloc(strlen(p)+1);
          pi->next=NULL;pi->offs=0;pi->cmdnum=-1;pi->size=0;
          if (!optimizing){ pi->sline=tfs->lines;pi->fname=tfs->FileName;}
                     else { pi->sline=c_del->sline;pi->fname=c_del->fname;}
          if (pi->pnum==-1) pi->cmdnum=-2;pi->def_area=def_area;
          if ((pi->prefix==NULL&&*t!=0)||pi->command==NULL)
              error("memory allocation #4",efatal);
          strcpy(pi->prefix,t);strcpy(pi->command,p);
          instr++;

          // search for opcode in table
          if (pi->cmdnum==-1) for (a=_index[*pi->command];*(i386_optab[a].name)!=0;a++)
            if (strcmp(i386_optab[a].name,pi->command)==0&&
                 i386_optab[a].operands==pi->pnum){

              // Check for parameters
              b=0;pi->direction=0;
              for (c=0;c<pi->pnum;c++)
                if ((pi->ptype[c]&i386_optab[a].operand_types[c])==0) b=1;

              // Check for Direction present
              if (b==1&&(i386_optab[a].opcode_modifier&D)&&i386_optab[a].operands==2){
                b=0;pi->direction=1;
                if ((pi->ptype[0]&i386_optab[a].operand_types[1])==0) b=1;
                if ((pi->ptype[1]&i386_optab[a].operand_types[0])==0) b=1;
              }

              if (b==0){
                pi->cmdnum=a;
                break;
              }
          }

          if (pi->cmdnum==-1) error("invalid opcode",eerror);

          // DEBUG
          #ifdef _DEBUG_asm
            fprintf(stderr,"#code:(%i,%i):%s",pi->cmdnum,pi->direction,pi->command);
            for (a=0;a<pi->pnum;a++) fprintf(stderr,"_%s(%i)",pi->params[a],pi->ptype[a]);
            fprintf(stderr,"; (%s), line %i, file %s\n",pi->prefix,pi->sline,pi->fname);
          #endif

        } else error("space absent",eerror);
      };

      if(nlast){
        _ln=_line;_lt=_lntp;_ln++;_lt++;*_line=';';

        switch(nlast){
          case '{':
            _up_down[_deep]=_cnter;
            strcpy(t,"up_");p=t+3;a=_cnter;_cnter++;
            for (b=0;b<3;b++){ *p++='a'+a%26;a=a/26;} *p=0;
            if (*idname!=0) strcat(idname,",");
            strcat(idname,t);

            #ifdef _DEBUG_asm
              fprintf(stderr,"#label({): '%s'\n",t);
            #endif

            _deep++;
            if (_deep==max_deep) error("too much '{'",efatal);
            break;

          case '}':
            if (_deep==0) error("too much '}'",efatal);
            _deep--;

            strcpy(t,"down_");p=t+5;a=_up_down[_deep];
            for (b=0;b<3;b++){ *p++='a'+a%26;a=a/26;} *p=0;
            if (*idname!=0) strcat(idname,",");
            strcat(idname,t);

            #ifdef _DEBUG_asm
              fprintf(stderr,"#label(}): '%s'\n",t);
            #endif

            break;

        }
      }
    }
  }
}

void process()
{
  if (*_line==0||ifdef_lev!=0) return;
  ProcessDefines();

  #ifdef _DEBUG_process
    fprintf(stderr,"#process:%s=%s;\n",_line,_lntp);
  #endif

  asm_code();
}
