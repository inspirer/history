// color.h

class cl_config {
public: 

  int cl_standart;
  int cl_edit;
  int cl_currentedit;
  int cl_cureditsel;
  int cl_editsel;
  int cl_listinactive;
  int cl_listactive;

  void defvar(int varn,char *value);
  void readconfig(const char *n,const char *section);
  void writeconfig(const char *n,const char *section);
  void defaults();
};

