# configParserGenerator 1.2.1/Perl/16.07.01
#

$cfgh = "config.h";
$cfgc = "config.cpp";
$cini = "jMail.ini";
$ctmp = "jMail.tmp";
$class_name = "config";
$names_per_line = 5;
$errors = "errors";
$section = "Master";

# 
# 

$configcpp1=<<CONFIGCPP1;
// $cfgc

#include "stdafx.h"

#define var(c) ((c>='a'&&c<='z')||(c>='0'&&c<='9')||c=='_')
#define num(c) (c>='0'&&c<='9')

const static char* keywords[] = {
CONFIGCPP1
$configcpp2=<<CONFIGCPP2;
""
};

class errorsCfg {
public: char err[80];
  errorsCfg(const char *l){ strcpy(err,l); }
};

static int getfromint(char *val)
{
  int i = 0;
  while(num(*val)){ i*=10;i+=*val-'0';val++;}
  if (*val) throw errorsCfg("invalid number");
  return i;
}

static CString getfromCString(char *val)
{
  if(*val!='"'||val[1]==0||val[strlen(val)-1]!='"') throw errorsCfg("wrong string");
  val[strlen(val)-1]=0;return CString(val+1);
}

void ${class_name}::defvar(int varn,char *value)
{
  switch(varn){
CONFIGCPP2
$configcpp3=<<CONFIGCPP3;
  }
}

void ${class_name}::readconfig(const char *n,const char *section)
{
  FILE *f;
  char s[128],*h,*l;
  DWORD lines = 0,i,e,q=0;

  defaults();
  if ((f=fopen(n,"r"))==NULL){
    sprintf(s,"`%s' not found",n);
    throw $errors(s);
  }

  while (fgets(s,128,f)!=NULL){
    lines++;h=s;
    while(*h) if (*h=='#'||*h=='\\n') *h=0;else h++;
    h=s;while(*h==' ') h++;
    if (!*h) continue;
    if (*h=='['){ 
      l=h;while(*l) if (*l==']') *l=0;else l++;
      q=!strcmp(h+1,section);continue;
    }
    if(!q) continue;
    l=h;while(*l) l++;l--;
    while(l>h&&*l==' ') *l--=0;
    if (!var(*h)){
      sprintf(s,"%s(%i): wrong line",n,lines);
      throw $errors(s);    
    }
    l=h;while(var(*l)) l++;
    if (*l=='=') *l++=0;
    else {
      *l++=0;
      while(*l&&*l!='=') l++;
      if (!*l){
        sprintf(s,"%s(%i): wrong line",n,lines);
        throw $errors(s);    
      }
      *l++=0;
    }
    while(*l==' ') l++;
    for(i=e=0;*keywords[i];i++) if(!strcmp(h,keywords[i])){
      try {
      e=1;defvar(i,l);
      }
      catch(errorsCfg& e){
        sprintf(s,"%s(%i): %s",n,lines,e.err);
        throw $errors(s);
      }
    }
    if (!e){
      sprintf(s,"%s(%i): unknown parameter `%s'",n,lines,h);
      throw $errors(s);
    }
  }
  fclose(f);
}

void ${class_name}::writeconfig(const char *n,const char *section)
{
  FILE *f,*old=NULL;
  char s[30],w[128],*h;

  if(!rename(n,"$ctmp"))
    old=fopen("$ctmp","r");
  if ((f=fopen(n,"w"))==NULL){
    sprintf(w,"`%s' not created",n);
    throw $errors(w);
  }

  if (old){
    while(fgets(w,128,old)!=NULL){
      h=w;while(*h) if (*h=='\\n') *h=0;else h++;
      if (*w=='['){ h=w;while(*h) if (*h==']') *h=0;else h++; }
      if (*w=='['&&(!strcmp(w+1,section))){
        while(fgets(w,128,old)!=NULL){
          if (*w=='[') break;*w=0;
        }
        break;
      } else { fprintf(f,"%s\\n",w);}
    }
    fprintf(f,"[%s]\\n",section);
  } else {
    fprintf(f,"# %s\\n\\n[%s]\\n",n,section);
  }
  
CONFIGCPP3
$configcpp4=<<CONFIGCPP4;

  if(old){
    fprintf(f,"\\n");
    if (*w) fprintf(f,"%s",w);
    while (fgets(w,128,old)!=NULL) fprintf(f,"%s",w);
    fclose(old);
    unlink(\"$ctmp\");
  }
  fclose(f);
}

void ${class_name}::defaults()
{
CONFIGCPP4
$configcpp5=<<CONFIGCPP5;
}
CONFIGCPP5

$configh1=<<CONFIGH1;
// $cfgh

class $class_name {
public: 
CONFIGH1

$configh2=<<CONFIGH2;

  void defvar(int varn,char *value);
  void readconfig(const char *n,const char *section);
  void writeconfig(const char *n,const char *section);
  void defaults();
};
CONFIGH2

  @names=();@defs=();@setts=();@types=();

  #read variables
  FR: while(<DATA>){
    chomp;
    s/ *\|\|\|.*$//;
    if (/^([^,]+),(.*)$/){
       $name=$1;$temp=$2;
       if ($name =~ /^([^:]+):(.+)$/){
         $type=$2;$name=$1;
         if($temp =~ /^([^,]+),([^,]+)$/){
           $default=$1;$setting=$2;
         } else { print "error($.): $temp\n";next FR; }
       } elsif ($temp =~ /^(\"[^\"]*\"),(\"[^\"]*\")$/){
         $default=$1;$setting=$2;$type="CString";
       } elsif($temp =~ /^(\d+),(\d+)$/){
         $default=$1;$setting=$2;$type="int";
       } else {
         print "error($.): $temp\n";next FR;
       }
       $type =~ s/\./,/g;
       push @names,$name;push @types,$type;push @setts,$setting;push @defs,$default;
       
    } elsif(/^#/ || !($_)){
       push @names,$_;push @setts,"";push @types,0;push @defs,"";
    } else { print "error($.): '$_'\n"; }
  }
  $lines=$.;

  #save INI
  rename $cini,$ctmp;
  $oldexist = open(OLDINI,"< $ctmp");
  open(INI,"> $cini") or die "not created $cini: $!\n";
  if ($oldexist){
    $leftstring="";
    PJ: while(<OLDINI>){
      chomp;
      if (/^\[$section\]$/){
        LJ: while(<OLDINI>){
          chomp;
          if (/^\[.*\]$/){ $leftstring=$_;last LJ;}
        }
        last PJ;
      } else { print INI "$_\n";}
    }
    print INI "[$section]\n";
  } else {
    print INI "# $cini\n\n[$section]\n";
  }
  for($i=0;$i<=$#names;$i++){
    if ($types[$i]){
      print INI "$names[$i]=$setts[$i]\n";
    } else { print INI "$names[$i]\n"; }
  }
  print INI "\n";
  if($oldexist){
    print INI "$leftstring\n" if $leftstring;
    while(<OLDINI>){ print INI "$_";}
    close(OLDINI);
    unlink($ctmp);
  }
  close(INI) or die "not saved $cini\n";

  @xtype=();@xname=();

  #save HEADER
  open(CFG_H,"> $cfgh") or die "not created $cfgh: $!\n";
  print CFG_H $configh1;
  for ($i=0;$i<=$#names;$i++){
    if($types[$i] =~ /,/){
      push @xname,$names[$i];
      $prefix=$names[$i];$prefix =~ s/^(..).*/$1/;$prefix.="_";
      $new = $prefix.$types[$i];
      $new =~ s/,/,$prefix/g;
      $types[$i]=$new;push @xtype,$new;$new =~ s/,/=0,/;
      $defs[$i]=$prefix.$defs[$i];
      print CFG_H "  enum { $new };\n";
      print CFG_H "  int $names[$i];\n";
    } elsif ($types[$i]){
      push @xname,$names[$i];push @xtype,$types[$i];
      print CFG_H "  $types[$i] $names[$i];\n";
    } elsif(!($names[$i])) { print CFG_H "\n"; }
  }
  print CFG_H $configh2;
  close(CFG_H) or die "not saved $cfgh\n";

  #save CPP
  open(CFG_C,"> $cfgc") or die "not created $cfgc: $!\n";
  print CFG_C $configcpp1;
  for ($i=0,$e=1;$i<=$#xname;$i++,$e++){
    print CFG_C "\"$xname[$i]\",";
    print CFG_C "\n" if $e%$names_per_line == 0;
  }
  print CFG_C $configcpp2;
  for ($i=0;$i<=$#xname;$i++){
    $ctp = $xtype[$i];
    if ($ctp =~ /,/){
      print CFG_C "   case $i:\n";
      $e=0;
      for(split /,/,$ctp){
        ($tp = $_) =~ s/^.*_(.*)/$1/;
        if ($e!=0){ print CFG_C "    else ";} else { print CFG_C "    ";}
        print CFG_C "if (!strcmp(value,\"$tp\")) $xname[$i]=$_;\n";
        $e++;                           
      }
      print CFG_C "    else throw errorsCfg(\"invalid value\");break;\n";
    } else { print CFG_C "   case $i: $xname[$i]=getfrom$ctp(value);break;\n"; }
  }
  print CFG_C $configcpp3;
  for ($i=0;$i<=$#names;$i++){
    $ctp = $types[$i];
    if ($ctp){
      if ($ctp eq "int"){
        print CFG_C "  fprintf(f,\"$names[$i]=%i\\n\",$names[$i]);\n";
      } elsif ($ctp eq "CString"){
        print CFG_C "  fprintf(f,\"$names[$i]=\\\"%s\\\"\\n\",$names[$i]);\n";
      } else {
        print CFG_C "  switch($names[$i]){\n";
        @tlist =  split(/,/,$ctp);
        for(@tlist){
          ($tp = $_) =~ s/^.*_(.*)/$1/;
          print CFG_C "    case $_: strcpy(s,\"$tp\");break;\n";
        }
        print CFG_C "  } fprintf(f,\"$names[$i]=%s\\n\",s);\n";
      }
    } else { print CFG_C "  fprintf(f,\"$names[$i]\\n\");\n"; }
  }
  print CFG_C $configcpp4;
  for ($i=0;$i<=$#names;$i++){
    if ($types[$i]){
      $defs[$i] =~ s/(.)([\"\\\'].)/$1\\$2/g;
      print CFG_C "  $names[$i]=$defs[$i];\n";
    } elsif(!($names[$i])) { print CFG_C "\n"; }
  }
  print CFG_C $configcpp5;
  close(CFG_C) or die "not saved $cfgc\n";

  #done
  print "configParserGenerator: $lines lines, @{[$#xname+1]} variables done\n";

__DATA__

address,"unknown","2:5030/611.23"
sysop,"unknown","Eugeniy Gryaznov"
bbsname,"bbs","ChaOS inspirer station"
phone,"-Unpublished-","(812)246-1049"
place,"unknown","St.Petersburg"
flags,"","MO,XX,V34"
timezone,0,4
inbound,"","f:\fido\inbox\"
inboundunsec,"","f:\fido\inbox\"
log,"","f:\temp\jmail.log"
chat_log,"","f:\temp\chat.log"

comport,"com2","com2"
baudrate,57600,57600
min_baud_in,9600,9600
min_baud_out,9600,9600
waitmdmanswer,4,4          ||| sec
modem_delay,2,2            ||| 1/10 sec
init_interval,20,20        ||| min
init1,"ATZ|","ATZ|"
init2,"","ATX3|"
ok,"OK","OK"
busy,"BUSY","BUSY"
connect,"CONNECT","CONNECT"
no_dial,"NO DIAL TONE","NO DIAL TONE|NO DIALTONE|LINE IN USE"
ring,"RING","RING"
ncarrier,"NO CARRIER","NO CARRIER|NO ANSWER"
dial_terminate,"v''^'|","v''^'|"
dial_prefix,"ATD","ATDP"
dial_suffix,"|","|"
onhook,"~v~~^~ATH0|","~v~~^~ATH0|"
modem_answer,"ATA|","ATA|"
error_correction,"V42|MNP|COMP","V42|MNP|ARQ|.42|X.|REL|ALT|PEP|HST|LAP|COMP"

time_dial,60,60
call_tries,15,900
circle,10,10
call_time,"0:00-24:00","11:00-18:00"

answer,"","11:00-18:00"
emsi_oh,"0:00-0:00","00:00-24:00"
answer_ring,0,2
wait_carrier,50,50
protocol_use:hydra.zmodem,hydra,hydra
