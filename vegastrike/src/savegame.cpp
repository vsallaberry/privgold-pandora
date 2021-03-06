#include <Python.h>
#include <sstream>
#include "cmd/unit_generic.h"
#include "hashtable.h"
#include <float.h>
#include "vsfilesystem.h"
#include <vector>
#include <string>
#include "configxml.h"
#include "vs_globals.h"
#include "savegame.h"
#include "load_mission.h"
#include <algorithm>
#include "cmd/script/mission.h"
#include "gfx/cockpit_generic.h"
#include "networking/const.h"
#include "vsfilesystem.h"
#include "cmd/fg_util.h"
#include "log.h"
#include "unicode.h"
#include "vs_log_modules.h"
#include "options.h"

extern vs_options game_options;

using namespace std;
using namespace VSFileSystem;
std::string CurrentSaveGameName="";
std::string GetHelperPlayerSaveGame (int num)
{
	if( Network==NULL)
	{
		if (CurrentSaveGameName.length()>0){
			if (game_options.remember_savegame) {
				VSFile f;
				VSError err = f.OpenCreateWrite( "save.4.x.txt", UnknownFile);
				if (err<=Ok) {
					f.Write(CurrentSaveGameName);
					f.Close();
				}		
			}
			if (num!=0) {
				return CurrentSaveGameName+XMLSupport::tostring(num);
			}
			return CurrentSaveGameName;
		}
      GAME_LOG(logvs::NOTICE, "Hi helper play %d", num);
	  static string *res=NULL;
	  if (res==NULL) {
		res = new std::string;
		//char c[2]={'\0','\0'};
		VSFile f;
		// TRY TO OPEN THE save.4.x.txt FILE WHICH SHOULD CONTAIN THE NAME OF THE SAVE TO USE
		VSError err = f.OpenReadOnly( "save.4.x.txt", UnknownFile);
		if (err>Ok) {
		  // IF save.4.x.txt DOES NOT EXIST WE CREATE ONE WITH "default" AS SAVENAME
		  err = f.OpenCreateWrite( "save.4.x.txt", UnknownFile);
		  if (err<=Ok) {
			f.Write(game_options.new_game_save_name.c_str(),game_options.new_game_save_name.length());
			f.Write("\n",1);
			f.Close();
		  }
		  else
		  {
	  		GAME_LOG(logvs::ERROR, "!!! ERROR : Creating default save.4.x.txt file : %s", f.GetFullPath().c_str());
			exit(1);
		  }
		  err = f.OpenReadOnly( "save.4.x.txt", UnknownFile);
		  if( err>Ok)
		  {
	  		GAME_LOG(logvs::ERROR, "!!! ERROR : Opening the default save we just created");
			exit(1);
		  }
		}
		if (err<=Ok) {
		  long length=f.Size();
		  if (length>0) {
		  char * temp = (char *)malloc (length+1);
		  temp[length]='\0';
		  f.Read (temp,length);
		  bool end=true;
		  for (int i=length-1;i>=0;i--) {
			if (temp[i]=='\r'||temp[i]=='\n') {
			  temp[i]=(end?'\0':'_');
			}else if (temp[i]=='\0'||temp[i]==' '||temp[i]=='\t') {
			  temp[i]=(end?'\0':'_');
			}else {
			  end=false;
			}
		  }
		  *res = (temp);
		  free (temp);
		  }
		  f.Close();

		}
	#if 0
		if (err<=Ok) {
		while (!f.Eof()) {
		  f.Read( &c[0], sizeof( char));
		  if (!f.Eof()) {
			if (c[0]!='\r'&&c[0]!='\n'&&c[0]!='\0') {
			  if (c[0]==' ') {
				c[0]='_';
				if (f.Eof()) {
				  continue;
				}
			  }
			  (*res)+=c;
			}
		  }else {
			break;
		  }
		}
		f.Close();
	#endif
		if (game_options.remember_savegame && !res->empty())
		{
		  // Set filetype to Unknown so that it is searched in homedir/
		  if (*res->begin()=='~')
		  {
			  err = f.OpenCreateWrite( "save.4.x.txt", VSFileSystem::UnknownFile);
			if (err<=Ok) {
	  		for (unsigned int i=1;i<res->length();i++)
			{
				char cc = *(res->begin()+i);
	    		f.Write ( &cc,sizeof(char));
	  		}
	 		char cc=0;
	  		f.Write (&cc,sizeof(char));
	  		f.Close();
		  }
		}
	  } 

	    
	#if 0
		err = f.OpenReadOnly( "save.4.x.txt", SaveFile);
		if (err<=Ok) {
		  char cc=0;
		  f.Write(&cc,sizeof( char));
		  f.Close();
		}
	#endif
	  }
	  if (num==0||res->empty()) {
		GAME_LOG(logvs::NOTICE, "Savegame Here");
		return (*res);  
	  }
	  return (*res)+XMLSupport::tostring(num);
	}
	else
	{
		//return Network[num].getCallsign();
		// Return "" so that the filename argument to ParseSavegame will be used
		return "";
	}
}

std::string GetWritePlayerSaveGame(int num) {
  string ret = GetHelperPlayerSaveGame(num);
  if (!ret.empty()) {
    if (*ret.begin()=='~') {
      return ret.substr (1,ret.length());
    }
  }
  return ret;
}

std::string GetReadPlayerSaveGame(int num) {
  string ret = GetHelperPlayerSaveGame(num);
  if (!ret.empty()) {
    if (*ret.begin()=='~') {
      return "";
    }
  }
  return ret;
}

// Used only to copy a savegame to a different named one
bool SaveFileCopy (const char * src, const char * dst) {
  if (dst[0]!='\0'&&src[0]!='\0') {

    VSFile f;
    VSError err = f.OpenReadOnly( src, SaveFile);
    if (err<=Ok) {
      string savecontent = f.ReadFull();
	  f.Close();
      err = f.OpenCreateWrite( dst, SaveFile);
      if (err<=Ok) {
		f.Write( savecontent);
	    f.Close();
      }
	  else {
	  	GAME_LOG(logvs::WARN, "WARNING : couldn't open savegame to copy to : %s as SaveFile", dst);
	  	return false;
	  }
    }
    else {
  	  GAME_LOG(logvs::WARN, "WARNING : couldn't find the savegame to copy : %s as SaveFile", src);
  	  return false;
    }
  }
  return true;
}

class MissionStringDat {
public:
  //typedef vsUMap<string,vector <StringPool::Reference> >MSD;
  typedef std::map<string,vector <std::string> >MSD;
  MSD m;
};
class MissionFloatDat {
public:
  typedef vsUMap<string,vector <float> >MFD;
  MFD m;
};
SaveGame::SaveGame(const std::string &pilot) {
  callsign=pilot;
  ForceStarSystem=string("");
  PlayerLocation.Set(FLT_MAX, FLT_MAX,FLT_MAX);
  missionstringdata = new MissionStringDat;
  missiondata = new MissionFloatDat;

}
SaveGame::~SaveGame() {
  delete missionstringdata;
  delete missiondata;
}
void SaveGame::SetStarSystem (string sys) {
  ForceStarSystem = sys;
}
string SaveGame::GetStarSystem () {
  return ForceStarSystem;
}
string SaveGame::GetOldStarSystem () {
  return ForceStarSystem;
}

void SaveGame::SetPlayerLocation (const QVector &v) {
  //VSFileSystem::vs_fprintf (stderr,"Set Location %f %f %f",v.i,v.j,v.k);
  if ((FINITE (v.i)&&FINITE(v.j)&&FINITE(v.k))) {
    PlayerLocation =v;
  }else {
    GAME_LOG(logvs::ERROR, "NaN ERROR saving unit");
    assert(FINITE (v.i)&&FINITE(v.j)&&FINITE(v.k));
    PlayerLocation.Set(1,1,1);
  }
}
QVector SaveGame::GetPlayerLocation () {
  return PlayerLocation;
}

void SaveGame::RemoveUnitFromSave (void * address) {
  /*
  SavedUnits *tmp;
  if (NULL!=(tmp =savedunits->Get (address))) {
    savedunits->Delete (address);
    delete tmp;
  }
  */
}
string SaveGame::WriteNewsData () {
  string ret("");
  int i = 0;
  vector <gameMessage> tmp;
  vector <string> newsvec;
  newsvec.push_back ("news");
  //for (gameMessage last; (mission->msgcenter->last(i,last,newsvec)); ++i) { tmp.push_back (last); }
  for (MessageCenter::last_whoiterator it
	   = mission->msgcenter->last_begin(MessageWhoStrPredicate(newsvec)); !it.end(); ++it, ++i) {
    tmp.push_back (*it);
  }
  ret += XMLSupport::tostring(i)+"\n";
  for (int j=tmp.size()-1;j>=0;j--) {
    char * msg = strdup (tmp[j].message.get().c_str());
    int k=0;
    while (msg[k]) {
      if (msg[k]=='\r')
	msg[k]=' ';
      if (msg[k]=='\n')
	msg[k]='/';
      k++;
    }
    ret += string( msg)+"\n";
    free (msg);
  }
  return ret;
}
std::string FixEncodingInString(const std::string & str) {
    static bool fix_encoding = XMLSupport::parse_bool(vs_config->getVariable(
                                    "general", "fix_savegame_encoding_on_load", "true"));
    if ( ! fix_encoding) {
        return str;
    }
    std::string fixed;
    fixed.reserve(str.length()+20);
    for (Utf8Iterator it = Utf8Iterator::begin(str); it != it.end(); ++it) {
        char utf8[MB_CUR_MAX+1];
        if (*it && utf32_to_utf8(utf8, *it) > 0) {
            fixed.append(utf8);
        } else {
            GAME_LOG(logvs::WARN, "empty character (end:%d)", it == it.end());
        }
    }
    if (strcmp(fixed.c_str(), str.c_str())) {
        GAME_LOG(logvs::INFO, "The SaveGame data '%s' has been fixed.", fixed.c_str());
    }
    return fixed;
}
vector <string> parsePipedString(string s) {
  string::size_type loc;
  vector <string> ret;
  while ((loc = s.find("|"))!=string::npos) {
	std::string sub = s.substr (0,loc);
    ret.push_back(FixEncodingInString(sub));
	GAME_LOG(logvs::NOTICE, "Found ship named : %s",s.substr( 0, loc).c_str());
    s = s.substr (loc+1);
  }
  if (s.length())
  {
    ret.push_back(FixEncodingInString(s));
	GAME_LOG(logvs::NOTICE, "Found ship named : %s", s.c_str());
  }
  return ret;
}
string createPipedString(vector <string> s) {
  string ret;
  for (unsigned int i=0;i<s.size()-1;i++) {
    ret += s[i]+"|";
  }
  if (s.size())
    ret+=s.back();
  return ret;
}
bool CopySavedShips(std::string filename, int player_num, const std::vector<std::string> &starships, bool load) {
  Cockpit * cp = _Universe->AccessCockpit(player_num);
  for (int i=0;i<starships.size();i+=2) {
    if (i==2) i=1;
    VSFile src,dst;
    string srcnam=filename;//cp->GetUnitModifications();
    string dstnam= GetWritePlayerSaveGame(player_num);
    string tmp;
    if (load) {
      tmp=srcnam;
      srcnam=dstnam;
      dstnam=tmp;
    }
    VSError e = src.OpenReadOnly(srcnam+"/"+starships[i]+".csv",UnitSaveFile);
    if (e<=Ok) {

      VSFileSystem::CreateDirectoryHome (VSFileSystem::savedunitpath+"/"+dstnam);
      VSError f = dst.OpenCreateWrite(dstnam+"/"+starships[i]+".csv",UnitFile);
      if (f<=Ok) {
        string srcdata=src.ReadFull();
        dst.Write(srcdata);
      }else {
        GAME_LOG(logvs::ERROR, "Error: Cannot Copy Unit %s from save file %s to %s",
                    starships[i].c_str(),
                srcnam.c_str(),
                dstnam.c_str());
        return false;
      }
    }else {
      GAME_LOG(logvs::ERROR, "Error: Cannot Open Unit %s from save file %s.",
              starships[i].c_str(),
              srcnam.c_str());
      return false;
    }        
  }
  return true;
}
bool WriteSaveGame (Cockpit * cp,bool auto_save) {
  int player_num= 0;
  for (int kk=0;kk<_Universe->numPlayers();++kk) {
    if (_Universe->AccessCockpit(kk)==cp)
      player_num=kk;
  }
  Unit * un = cp->GetSaveParent();
  if (!un) {
    return true;
  }
  if (un->GetHull()>0) {
    GAME_LOG(logvs::NOTICE, "WriteSaveGame '%s' (auto:%d)", GetCurrentSaveGame().c_str(), auto_save);
    if (cp->savegame->WriteSaveGame (cp->activeStarSystem->getFileName().c_str(),
    		                         un->LocalPosition(),cp->credits,cp->unitfilename,auto_save?-1:player_num).empty()) {
    	return false;
    }
    if (un->WriteUnit(cp->GetUnitModifications().c_str()) != true) {
    	return false;
    }
    if (GetWritePlayerSaveGame(player_num).length()&&!auto_save) {
      cp->savegame->SetSavedCredits (_Universe->AccessCockpit()->credits);
      cp->savegame->SetStarSystem(cp->activeStarSystem->getFileName());
      //      un->WriteUnit(GetWritePlayerSaveGame(player_num).c_str());
      cp->savegame->SetPlayerLocation(un->LocalPosition());    
      if (CopySavedShips(cp->GetUnitModifications(),player_num,cp->unitfilename,false) != true) {
    	  return false;
      }
    }
  }
  return true;
}
int hopto (char *buf,char endln, char endln2,int readlen) {
  if (endln==' '||endln2==' ') {
    while (buf[readlen]&&buf[readlen]==' ') {
      readlen++;
    }
  }
  for(; buf[readlen]!=0&&buf[readlen]!=endln&&buf[readlen]!=endln2; readlen++) ;
  if (buf[readlen]&&(buf[readlen]==endln||buf[readlen]==endln2))
    readlen++;
  return readlen;
}

void SaveGame::ReadNewsData (char * &buf, bool just_skip) {
  int numnews;
  int i=0;
  vector <string> n00s;
  n00s.push_back ("news");
  vector<string> nada;
  mission->msgcenter->clear (n00s,nada);  
  int offset=hopto (buf,'\n','\n',0);
  if (offset>0) {
    //fgets (news,1023,fp);
    sscanf (buf,"%d\n",&numnews);
    buf+=offset;
    for (i=0;i<numnews;i++) {
      offset = hopto (buf,'\n','\n',0);
      if (offset>0)
    	  buf[offset-1]=0;
      int l=0;
      for (l=0;l<offset-1;l++) {
    	  if (buf[l]!='\r'&&buf[l]!='\n') {
    		  break;
    	  }
      }
      if (!just_skip&&buf[l]!='\r'&&buf[l]!='\n'&&buf[l]) {
    	  mission->msgcenter->add ("game","news",FixEncodingInString(std::string(buf+l)));
    	  GAME_LOG(logvs::VERBOSE, "NEWS(#%d): %s", i, buf+l);
      }
      buf+=offset;
    }
  }
}
void SaveGame::AddUnitToSave (const char * filename, int type, const char * faction, void * address) 
{
  if(game_options.Drone.compare(filename)){
    RemoveUnitFromSave (address);
    //    savedunits->Put (address,new SavedUnits (filename,type,faction));//not no more
  }
}
std::vector<float> &SaveGame::getMissionData(const std::string &magic_number) 
{
  return missiondata->m[magic_number];
}
unsigned int SaveGame::getMissionDataLength(const std::string &magic_number) const
{
  MissionFloatDat::MFD::const_iterator it = missiondata->m.find(magic_number);
  return (it == missiondata->m.end()) ? 0 : it->second.size();
}
std::vector<string> &SaveGame::getMissionStringData(const std::string &magic_number) 
{
  return missionstringdata->m[magic_number];
}
unsigned int SaveGame::getMissionStringDataLength(const std::string &magic_number) const
{
  MissionStringDat::MSD::const_iterator it = missionstringdata->m.find(magic_number);
  return (it == missionstringdata->m.end()) ? 0 : it->second.size();
}
template <class MContainerType> void RemoveEmpty (MContainerType &t) {
  typename MContainerType::iterator i;
  for (i=t.begin();i!=t.end();)
	  if (i->second.empty())
		  t.erase(i++); else
		  ++i;
}
string SaveGame::WriteMissionData () {
  string ret(" ");
  RemoveEmpty<MissionFloatDat::MFD> (missiondata->m);
  ret+=XMLSupport::tostring ((int)missiondata->m.size());
  for( MissionFloatDat::MFD::iterator i=missiondata->m.begin();i!=missiondata->m.end();i++) {
    unsigned int siz = (*i).second.size();
    ret += string("\n")+(*i).first+string(" ")+XMLSupport::tostring(siz)+" ";
    for (unsigned int j=0;j<siz;j++) {
      ret += XMLSupport::tostring((*i).second[j])+" ";
    }
  }
  return ret;
}
std::string scanInString (char * &buf) {
  std::string str;
  char c[2]={'\n','\0'};
  while (*buf&&isspace (*buf)) {
    buf++;
  }
  c[0]=*buf;
  while (*buf&&(!isspace (*buf))) {
    c[0]=*buf;
    str+=c;
    buf++;
  }
  return FixEncodingInString(str);
}
void SaveGame::ReadMissionData (char * &buf, bool select_data, const std::set<std::string> &select_data_filter) {
  missiondata->m.clear();
  int mdsize;
  char * buf2 = buf;
  sscanf (buf2," %d ",&mdsize);
  // Put ptr to point after the number we just read
  buf2 +=hopto (buf2,' ','\n',0);
  for( int i=0;i<mdsize;i++) {
    int md_i_size;
    string mag_num(scanInString (buf2));
    sscanf (buf2,"%d ",&md_i_size);
    // Put ptr to point after the number we just read
    buf2 +=hopto (buf2,' ','\n',0);
	vector <float> * vecfloat=0;
	bool skip=true;
	if (!select_data || select_data_filter.count(mag_num)) {
		missiondata->m[mag_num] = vector<float>();
		vecfloat=&missiondata->m[mag_num];
		skip=false;
	}
	for (int j=0;j<md_i_size;j++) {
	  if (!skip) {
	    double float_val;
		sscanf (buf2,"%lf ",&float_val);
		vecfloat->push_back (float_val);
	  }
	  // Put ptr to point after the number we just read
	  buf2 +=hopto (buf2,' ','\n',0);
	}
  }
  buf = buf2;
}
string AnyStringScanInString (char * &buf) {
  unsigned int size=0;
  bool found=false;
  while ((*buf)&&((*buf)!=' '||(!found))) {
    if ((*buf)>='0'&&(*buf)<='9') {
      size*=10;
      size+=(*buf)-'0';
      found=true;
    }
    buf++;
  }
  if (*buf)
    buf++;
  string ret;
  ret.resize(size);
  unsigned int i=0;
  while (i<size&&*buf) {
    ret[i++]=*(buf++);
  }
  return FixEncodingInString(ret);
}
void AnyStringSkipInString (char * &buf) {
  unsigned int size=0;
  bool found=false;
  while ((*buf)&&((*buf)!=' '||(!found))) {
    if ((*buf)>='0'&&(*buf)<='9') {
      size*=10;
      size+=(*buf)-'0';
      found=true;
    }
    buf++;
  }
  if (*buf)
    buf++;
  unsigned int i=0;
  while (i<size&&*buf) ++i,++buf;
}
string AnyStringWriteString (string input) {
  return XMLSupport::tostring ((int)input.length())+" "+input;
}
void SaveGame::ReadMissionStringData (char * &buf, bool select_data, const std::set<std::string> &select_data_filter) {
  missionstringdata->m.clear();
  int mdsize;
  char * buf2 = buf;
  sscanf (buf2," %d ",&mdsize);
  // Put ptr to point after the number we just read
  buf2 +=hopto (buf2,' ','\n',0);
  for( int i=0;i<mdsize;i++) {
    int md_i_size;
    string mag_num(AnyStringScanInString (buf2));
    //sscanf (buf2,"%d ",&md_i_size);
	md_i_size = strtol(buf2,(char **)NULL,10);
    // Put ptr to point after the number we just read
    buf2 +=hopto (buf2,' ','\n',0);
	vector <string> * vecstring = 0;
	bool skip=true;
	if (!select_data || select_data_filter.count(mag_num)) {
		missionstringdata->m[mag_num] = vector<string>();
		vecstring=&missionstringdata->m[mag_num];
		skip=false;
	}
    for (int j=0;j<md_i_size;j++) {
		if (skip)
			AnyStringSkipInString(buf2); else
			vecstring->push_back (AnyStringScanInString(buf2));
    }
  }
  buf = buf2;
  this->PurgeZeroStarships();
}
void SaveGame::PurgeZeroStarships() {
  for (MissionStringDat::MSD::iterator i=missionstringdata->m.begin(),ie=missionstringdata->m.end();i!=ie;++i) {
    if (fg_util::IsFGKey(i->first))
      if (fg_util::CheckFG(i->second)) {
        GAME_LOG(logvs::INFO, "correcting flightgroup %s to have right landed ships",i->first.c_str());
     
      }
  }
}


static inline void PushBackFloat(float f, vector <char> &ret) {
	char c[128];
    sprintf(c,"%f",f);
	char * k=&c[0];
	while (*k) {
		ret.push_back(*k);
	}
}
static inline void PushBackUInt(unsigned int i, vector<char> & ret) {
    char tmp[32];
    if (!i) {
		ret.push_back('0');
    } else {
        unsigned int p=0,q=ret.size();
        while(i) { tmp[p++]=(i%10+'0'); i/=10; }
        ret.resize(q+p);
        while(p) ret[q++]=tmp[--p];
	}
}
static inline void PushBackInt(int i, vector<char> &ret) {
	if (i<0)
		ret.push_back('-');
	PushBackUInt(i<0?-i:1,ret);
}
static inline void PushBackChars(const char * c,vector<char> & ret) {
    int ini=ret.size();
    ret.resize(ret.size()+strlen(c));
    while (*c) ret[ini++]=*(c++);
}
static inline void PushBackString (const string &input,vector<char> &ret) {
	PushBackUInt(input.length(),ret);
	PushBackChars(" ",ret);
	PushBackChars(input.c_str(),ret);
}
void SaveGame::WriteMissionStringData (std::vector <char> & ret) {
  RemoveEmpty<MissionStringDat::MSD> (missionstringdata->m);
  PushBackUInt(missionstringdata->m.size(),ret);
  for( MissionStringDat::MSD::iterator i=missionstringdata->m.begin();i!=missionstringdata->m.end();i++) {
    const string &key = (*i).first;
    unsigned int siz = (*i).second.size();
    if (key == "mission_descriptions" || key == "mission_scripts" || key == "mission_vars" || key == "mission_names") {
		// *** BLACKLIST ***
		// Don't bother to write these out since they waste a lot of space and aren't used.
		siz = 0; // Not writing them out altogether will cause saved games to break.
	}
	PushBackChars("\n",ret);
	PushBackString(key,ret);
	PushBackUInt(siz,ret);
	PushBackChars(" ",ret);
    for (unsigned int j=0;j<siz;j++) {
		PushBackString((*i).second[j],ret);
    }
  }
}
/*static inline void SerializeString(std::string str, std::string &outstr) {
    char tmp[32];
    sprintf(tmp,"%u ",str.length());
    outstr += tmp;
    outstr += str;
}

std::string SaveGame::WriteMissionStringData () {
  char tmp[32];
  string ret;

  RemoveEmpty<MissionStringDat::MSD> (missionstringdata->m);
  sprintf(tmp,"%u",missionstringdata->m.size());
  ret = tmp;
  for( MissionStringDat::MSD::iterator i=missionstringdata->m.begin();i!=missionstringdata->m.end();i++) {
    unsigned int siz = (*i).second.size();
    ret += "\n";
    SerializeString((*i).first,ret);
    sprintf(tmp,"%u ",siz);
    ret += tmp;
    for (unsigned int j=0;j<siz;j++)
        SerializeString((*i).second[j],ret);
  }

  return ret;
}*/

void SaveGame::ReadStardate( char * &buf)
{
	string stardate( AnyStringScanInString( buf));
	GAME_LOG(logvs::NOTICE, "Read stardate : %s", stardate.c_str());
	_Universe->current_stardate.InitTrek( stardate);
}

void SaveGame::ReadSavedPackets (char * &buf, bool commitfactions, bool skip_news, bool select_data, const std::set<std::string> &select_data_filter) {
  int a=0;
  char unitname[1024];
  char factname[1024];
  while (3==sscanf (buf,"%d %s %s",&a,unitname,factname)) {
	// Put i to point after what we parsed (on the 3rd space read)
    while ((*buf)&&isspace(*buf))++buf;
        buf+=hopto (buf,' ','\n',0);
        buf+=hopto (buf,' ','\n',0);
        buf+=hopto (buf,' ','\n',0);
    if (a==0&&0==strcmp(unitname,"factions")&&0==strcmp(factname,"begin")) {
      if (commitfactions) FactionUtil::LoadSerializedFaction(buf);

      break;//GOT TO BE THE LAST>... cus it's stupid :-) and mac requires the factions to be loaded AFTER this function call
    }else if (a==0&&0==strcmp(unitname,"mission")&&0==strcmp(factname,"data")) {
      ReadMissionData(buf,select_data,select_data_filter);
    }else if (a==0&&0==strcmp(unitname,"missionstring")&&0==strcmp(factname,"data")) {
      ReadMissionStringData(buf,select_data,select_data_filter);
    }else if (a==0&&0==strcmp(unitname,"python")&&0==strcmp(factname,"data")) {
      last_written_pickled_data=last_pickled_data=UnpickleAllMissions(buf);
    }else if (a==0&&0==strcmp(unitname,"news")&&0==strcmp(factname,"data")) {
      if (commitfactions) ReadNewsData(buf,skip_news);
    }else if (a==0&&0==strcmp(unitname,"stardate")&&0==strcmp(factname,"data")) {
	  // On server side we expect the latest saved stardate in dynaverse.dat too
      if (commitfactions&&!SERVER/*server never wants to take "orders" from shapeshifters...*/)
        ReadStardate(buf);
      else
        AnyStringScanInString(buf);
    }else {
      char output[31]={0};
      strncpy(output, buf, 30);
      GAME_LOG(logvs::NOTICE, "buf unrecognized %s...",output);
      //su.push_back (SavedUnits (unitname,(clsptr)a,factname));
    }
  }
  GAME_LOG(logvs::NOTICE, "Exiting ReadSavedPackets");
}

void SaveGame::LoadSavedMissions() {
  unsigned int i;
  vector<string> scripts = getMissionStringData("active_scripts");
  vector<string> missions = getMissionStringData("active_missions");
  if (PyRun_SimpleString("import VS\nVS.loading_active_missions=True\n"
                         "VS.Log('game', 4, \"Loading active missions \"+str(VS.loading_active_missions))\n")
      !=0) GAME_LOG(logvs::WARN, "cannot set loading_active_missions python flag");
  // kill any leftovers so they don't get loaded twice.
  Mission *ignoreMission = Mission::getNthPlayerMission(_Universe->CurrentCockpit(), 0);
  for (i=active_missions.size()-1;i>0;--i){// don't terminate zeroth mission
    if (active_missions[i]->player_num==_Universe->CurrentCockpit() &&
        active_missions[i]!=ignoreMission)
      active_missions[i]->terminateMission();
  }
  
  for (i=0;i<scripts.size()&&i<missions.size();++i) {
    try {
      if (!missions[i].empty()) // Built-in/networking missions
        LoadMission(missions[i].c_str(),scripts[i],false);
    }catch (...) {
      if (PyErr_Occurred()) {
        PyErr_Print();
        PyErr_Clear();
        fflush(stderr);         
        fflush(stdout);
      }else throw;    
      
    }
    
  }
  if (PyRun_SimpleString("import VS\nVS.loading_active_missions=False\n"
                         "VS.Log('game', 4, \"Loading active missions \"+str(VS.loading_active_missions))\n")
      !=0) GAME_LOG(logvs::WARN, "cannot restore loading_active_missions python flag");
  getMissionStringData("active_scripts")=scripts;
  getMissionStringData("active_missions")=missions;
}
string SaveGame::WriteSavedUnit (SavedUnits* su) {
  return string("\n")+XMLSupport::tostring(su->type)+string(" ")+su->filename+" "+su->faction;
}
 extern bool STATIC_VARS_DESTROYED;

 /*static char * tmprealloc (char * var, int &oldlength, int newlength) {
  if (oldlength<newlength) {
    oldlength= newlength;
    var= (char *)realloc(var,newlength);
  }
  memset (var,0,newlength);
  return var;
}*/

string SaveGame::WritePlayerData( const QVector &FP, std::vector<std::string> unitname, const char * systemname, float credits, std::string fact)
{
	std::ostringstream playerdata_oss;
	playerdata_oss.unsetf(std::ostringstream::floatfield); // cleared -> equivalent to %g
	playerdata_oss.setf(std::ostringstream::fixed); // equivalent to %f
	///*sprintf-version*/ string playerdata("");
	///*sprintf-version*/   int MB = MAXBUFFER;
	///*sprintf-version*/   char * tmp=(char *)malloc (MB);
	///*sprintf-version*/   memset( tmp, 0, MB);

	QVector FighterPos= PlayerLocation-FP;
	//    if (originalsystem!=systemname) {
	FighterPos=FP;
	//    }
	string pipedunitname = createPipedString(unitname);
	///*sprintf-version*/ tmp = tmprealloc(tmp,MB,pipedunitname.length()+strlen(systemname)+256/*4 floats*/);
	// If we specify no faction, it won't be saved in there
	playerdata_oss << systemname << "^" << (float)credits << "^" << pipedunitname << " "
				   << (float)FighterPos.i << " " << (float) FighterPos.j << " " << (float)FighterPos.k;
	if( !fact.empty()) {
		///*sprintf-version*/ sprintf (tmp,"%s^%f^%s %f %f %f %s",systemname,credits,pipedunitname.c_str(),FighterPos.i,FighterPos.j,FighterPos.k, fact.c_str());
		 playerdata_oss << " " << fact;
	} else {
		///*sprintf-version*/ sprintf (tmp,"%s^%f^%s %f %f %f",systemname,credits,pipedunitname.c_str(),FighterPos.i,FighterPos.j,FighterPos.k);
	}
	///*sprintf-version*/ playerdata = string( tmp);
	this->playerfaction = fact;
	SetSavedCredits (credits);
	///*sprintf-version*/ free(tmp);tmp=NULL;

	return playerdata_oss.str();
}

string SaveGame::WriteDynamicUniverse()
{
	///*sprintf-version*/ string dyn_univ("");
	std::ostringstream dyn_univ_oss;
  	///*sprintf-version*/ int MB = MAXBUFFER;
  	///*sprintf-version*/ char * tmp=(char *)malloc (MB);
  	///*sprintf-version*/ memset( tmp, 0, MB);

	// Write mission data
	// On server side we save the stardate
	if( SERVER)
	{
		GAME_LOG(logvs::NOTICE, "SAVING STARDATE - SERVER=%d", SERVER);
		dyn_univ_oss << "\n0 stardate data " << AnyStringWriteString( _Universe->current_stardate.GetFullTrekDate());
	}
    ///*sprintf-version*/ memset( tmp, 0, MB);
    ///*sprintf-version*/ sprintf (tmp,"\n%d %s %s",0,"mission","data ");
    ///*sprintf-version*/ dyn_univ += string( tmp);
	dyn_univ_oss << "\n" << 0 << " " << "mission" << " " << "data" << " ";
    dyn_univ_oss << WriteMissionData();
    ///*sprintf-version*/ memset( tmp, 0, MB);
    ///*sprintf-version*/ sprintf (tmp,"\n%d %s %s",0,"missionstring","data ");
    ///*sprintf-version*/ dyn_univ += string( tmp);
    dyn_univ_oss << "\n" << 0 << " " << "missionstring" << " " << "data" << " ";
	vector <char> missionstringdata1;
    WriteMissionStringData(missionstringdata1);
    dyn_univ_oss << string(&missionstringdata1[0],missionstringdata1.size());
		
    if (!STATIC_VARS_DESTROYED)
      last_written_pickled_data=PickleAllMissions(); 
    ///*sprintf-version*/ tmp = tmprealloc(tmp,MB,last_written_pickled_data.length()+256/*4 floats*/);
    ///*sprintf-version*/ sprintf (tmp,"\n%d %s %s %s ",0,"python","data",last_written_pickled_data.c_str());
    ///*sprintf-version*/ dyn_univ += string( tmp);
    dyn_univ_oss << "\n" << 0 << " " << "python" << " " << "data" << " " << last_written_pickled_data << " ";
	// Write news data
    ///*sprintf-version*/ memset( tmp, 0, MB);
    ///*sprintf-version*/ sprintf (tmp,"\n%d %s %s",0,"news","data ");
    ///*sprintf-version*/ dyn_univ += string( tmp);
    dyn_univ_oss << "\n" << 0 << " " << "news" << " " << "data" << " ";
    dyn_univ_oss << WriteNewsData();
	// Write faction relationships
    ///*sprintf-version*/ memset( tmp, 0, MB);
    ///*sprintf-version*/ sprintf (tmp,"\n%d %s %s",0,"factions","begin ");
    ///*sprintf-version*/ dyn_univ += string( tmp);
    dyn_univ_oss << "\n" << 0 << " " << "factions" << " " << "begin" << " ";
    dyn_univ_oss << FactionUtil::SerializeFaction();

	return dyn_univ_oss.str();
}

using namespace VSFileSystem;

string SaveGame::WriteSaveGame (const char *systemname, const QVector &FP, float credits, std::vector<std::string> unitname, int player_num, std::string fact, bool write) {
  savestring = string("");
    GAME_LOG(logvs::NOTICE, "Writing Save Game %s",outputsavegame.c_str());

	savestring += WritePlayerData( FP, unitname, systemname, credits, fact);
	savestring += WriteDynamicUniverse();
  if (outputsavegame.length()!=0) 
  {
    if( write)
	{
		VSFile f;
		VSError err = f.OpenCreateWrite( outputsavegame, SaveFile);
		if(err <= Ok)
		{// check
			// WRITE THE SAVEGAME TO THE MISSION SAVENAME
			f.Write( savestring.c_str(), savestring.length());
			f.Close();
			if (player_num!=-1)
			{// AND THEN COPY IT TO THE SPECIFIED SAVENAME (from save.4.x.txt)
				last_pickled_data =last_written_pickled_data;
				string sg =GetWritePlayerSaveGame(player_num);
				if (SaveFileCopy (outputsavegame.c_str(),sg.c_str()) != true)
					return std::string();
			}
		}
		else
		{// error occured while opening file
			GAME_LOG(logvs::WARN, "an error occured while opening file: %s", outputsavegame.c_str());
			return std::string();
		}
	}	
  }
  return savestring;
}

static float savedcredits=0;
float SaveGame::GetSavedCredits () {
		return savedcredits;
}
void SaveGame::SetSavedCredits (float c) {
		savedcredits = c;
}

void SaveGame::SetOutputFileName(const string &filename)
{
	if (!filename.empty())
		outputsavegame = string("Autosave-") + filename;
	else
		outputsavegame = string("Autosave"); // empty name?
}

void SaveGame::ParseSaveGame (const string &filename_p, string &FSS, const string &originalstarsystem, QVector &PP, bool & shouldupdatepos, float &credits, vector <string> &savedstarship, int player_num, const string &save_contents, bool read, bool commitfaction, bool quick_read, bool skip_news, bool select_data, const std::set<std::string> &select_data_filter) 
{
	const string &str = save_contents; // alias
	char *tempfullbuf=0;
	int tempfulllength=2048;
	string filename;
	/*
	// Formerly: filename = callsign + "_"
	if (!filename_p.empty())
		filename = filename_p;
	*/
	// Now leave filename empty, use the default name regardless...
    
	shouldupdatepos=!(PlayerLocation.i==FLT_MAX||PlayerLocation.j==FLT_MAX||PlayerLocation.k==FLT_MAX);
	// WE WILL ALWAYS SAVE THE CURRENT SAVEGAME IN THE MISSION SAVENAME (IT WILL BE COPIED TO THE SPECIFIED SAVENAME)
	SetOutputFileName(filename);
	VSFile f;
	VSError err=FileNotFound;
	if( read)
	{
		 {
				// TRY TO GET THE SPECIFIED SAVENAME TO LOAD
				string plsave = GetReadPlayerSaveGame(player_num);
             
				if (plsave.length()) {
                    GAME_LOG(logvs::NOTICE, "Loading Savegame '%s'...", plsave.c_str());
                    
					err = f.OpenReadOnly( plsave, SaveFile);
					if (err>Ok) { // failed in SaveFile
						// Try as an UnknownFile to get a datadir saved game, like New_Game.
						err = f.OpenReadOnly( plsave, UnknownFile);
					}
				}else if (filename.length()>0) {
                    GAME_LOG(logvs::NOTICE, "Loading Savegame '%s'...", filename.c_str());
                    
					// IF NONE SIMPLY LOAD THE MISSION DEFAULT ONE
					err = f.OpenReadOnly( filename, SaveFile);
				}
		}
		if( err<=Ok)
		{
			if (quick_read) {
				char *buf = (char*)malloc(game_options.quick_savegame_summaries_buffer+1);
				buf[game_options.quick_savegame_summaries_buffer]='\0';
				err = f.ReadLine(buf,game_options.quick_savegame_summaries_buffer);
				savestring = buf;
				free(buf);
			} else {
				savestring = f.ReadFull();
			}
		}
  }
  if( err<=Ok || (!read && str!=""))
  {
	  if (!read)
		savestring = str;
	  if ( savestring.length()>0) {
	    char * buf = new char[savestring.length()+1];
		buf[savestring.length()]='\0';
		memcpy( buf, savestring.c_str(), savestring.length());

		int headlen = hopto (buf,'\n','\n',0);
		char * deletebuf = buf;
		char * tmp2= (char *)malloc(headlen+2);
		char * freetmp2 = tmp2;
		char * factionname = (char *)malloc(headlen+2);
		if (headlen>0&&(buf[headlen-1]=='\n'||buf[headlen-1]==' '||buf[headlen-1]=='\r'))
			buf[headlen-1]='\0';
        factionname[headlen+1]='\0';
		QVector tmppos;
		int res = sscanf (buf,"%s %lf %lf %lf %s",tmp2,&tmppos.i,&tmppos.j,&tmppos.k, factionname);
		if (res==4 || res==5) {
		  // Extract credits & starship
		  for (int j=0;'\0'!=tmp2[j];j++) {
			if (tmp2[j]=='^') {
				sscanf (tmp2+j+1,"%f",&credits);
				tmp2[j]='\0';
				for (int k=j+1;tmp2[k]!='\0';k++) {
					if (tmp2[k]=='^') {
						tmp2[k]='\0';
						savedstarship.clear();
						savedstarship=parsePipedString(tmp2+k+1);
						break;
					}
				}
				break;
			}
		  }
		  // In networking save we include the faction at the end of the first line
		  if( res==5)
		  {
		  	playerfaction = string( factionname);
			GAME_LOG(logvs::NOTICE, "Found faction in save file : %s",playerfaction.c_str());
		  }
		  else
		  {
		    // If no faction -> default to privateer
		  	playerfaction = string( "privateer");
			GAME_LOG(logvs::NOTICE, "Faction not found assigning default one : privateer !!!");
		  }
		  free(factionname);

		  if (ForceStarSystem.length()==0)
			ForceStarSystem=string(tmp2);
		  if (PlayerLocation.i==FLT_MAX||PlayerLocation.j==FLT_MAX||PlayerLocation.k==FLT_MAX) {
			shouldupdatepos=true;
			PlayerLocation=tmppos;//LaunchUnitNear(tmppos);
		  }
		  buf+=headlen;
		  ReadSavedPackets (buf,commitfaction,skip_news,select_data,select_data_filter);
		}
		free(freetmp2);freetmp2=NULL;
		tmp2=NULL;
		delete []deletebuf;
	  }
	  if( read)
	  	f.Close();
  }

  if (PlayerLocation.i==FLT_MAX||PlayerLocation.j==FLT_MAX||PlayerLocation.k==FLT_MAX) {
	shouldupdatepos=false;
	PlayerLocation=PP;
  }else {
	PP = PlayerLocation;
	shouldupdatepos=true;
  }
  if (ForceStarSystem.length()==0) {
	ForceStarSystem = FSS;
	originalsystem=FSS;
  } else {
	originalsystem = ForceStarSystem;
	FSS = ForceStarSystem;
  }
  SetSavedCredits(credits);
  //  return mysav;
}

const string& GetCurrentSaveGame()
{
	return CurrentSaveGameName;
}

string SetCurrentSaveGame(string newname)
{
	string oldname = CurrentSaveGameName;
	CurrentSaveGameName = newname;
	return oldname;
}

const string& GetSaveDir()
{
	static string rv = VSFileSystem::homedir+"/save/";
	return rv;
}
