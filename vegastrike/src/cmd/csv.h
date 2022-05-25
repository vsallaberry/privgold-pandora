#include "config.h"
#include <string>
#include <vector>
#include <gnuhash.h>

#include "hashtable.h"
namespace VSFileSystem
{
	class VSFile;
}
//std::vector <std::string> readCSV(std::string line, std::string delim=",;");
std::vector <std::string> readCSV(const std::string & line, const std::string & delim=",;");
std::string writeCSV(const std::vector<std::string> &key, const std::vector<std::string> &table, const std::string & delim=",;");
class CSVTable {
 
private:
    void Init (const std::string & data);
public:
    std::string rootdir;
    vsUMap<std::string,int> columns;
    vsUMap<std::string,int> rows;
    std::vector<std::string> key;
    std::vector<std::string> table;

    CSVTable(const std::string & name, const std::string & saveroot);
    CSVTable(VSFileSystem::VSFile &f,  const std::string & saveroot);

    bool RowExists(const std::string & name, unsigned int&where);
    bool ColumnExists(const std::string & name, unsigned int&where);

public:
    //Optimizer toolbox
    enum optimizer_enum { optimizer_undefined=0x7fffffff };
    void SetupOptimizer(const std::vector<std::string> & keys, unsigned int type);

    //Opaque Optimizers - use the optimizer toolbox to set them up
    bool optimizer_setup;
    unsigned int optimizer_type;
    std::vector<std::string> optimizer_keys;
    std::vector<unsigned int> optimizer_indexes;
};

class CSVRow {
  std::string::size_type iter;
  CSVTable * parent;
public:
  std::string getRoot();
  size_t size() {return parent->key.size();}
  CSVRow(CSVTable * parent, const std::string & key);
  CSVRow(CSVTable * parent, unsigned int which);
  CSVRow(){parent=NULL;iter=std::string::npos;}
  const std::string& operator[](const std::string&) const;
  const std::string& operator[](unsigned int) const;
  const std::string& getKey(unsigned int which) const;
  std::vector<std::string>::iterator begin();
  std::vector<std::string>::iterator end();
  bool success()const {
    return parent!=NULL;
  }
  CSVTable* getParent() { return parent; };
};

extern std::vector<CSVTable*> unitTables;
