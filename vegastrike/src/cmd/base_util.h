#include "config.h"
#include <string>
#include <boost/version.hpp>
#if BOOST_VERSION != 102800
#include <boost/python/object.hpp>
#include <boost/python/dict.hpp>
typedef boost::python::dict BoostPythonDictionary ;
#else
#include <boost/python/objects.hpp>
typedef boost::python::dictionary BoostPythonDictionary ;
#endif

#if defined(HAVE_PYTHON)
	namespace boost { namespace python { class dict; } }
#else
	#include <map>
#endif

namespace BaseUtil {

#if defined(HAVE_PYTHON)
#if BOOST_VERSION != 102800
	typedef boost::python::dict Dictionary;
#else
	typedef boost::python::dictionary Dictionary;
#endif
#else
	typedef std::map<const std::string,const std::string> Dictionary;
#endif

	int Room (const std::string & text);
	void Texture(int room, const std::string & index, const std::string & file, float x, float y);
    void Video(int room, const std::string & index, const std::string & vfile, const std::string & afile, float x, float y);
    void VideoStream(int room, const std::string & index, const std::string & streamfile, float x, float y, float w, float h);
	void SetTexture(int room, const std::string & index, const std::string & file);
	void SetTextureSize(int room, const std::string & index, float w, float h);
	void SetTexturePos(int room, const std::string & index, float x, float y);
    void PlayVideo(int room, const std::string & index);
	void Ship (int room, const std::string & index,QVector pos,Vector R, Vector Q);
	void LinkPython (int room, const std::string & index,const std::string & pythonfile, float x, float y, float wid, float hei, const std::string & text, int to);
	void LaunchPython (int room, const std::string & index,const std::string & pythonfile, float x, float y, float wid, float hei, const std::string & text);
	void EjectPython (int room, const std::string & index,const std::string & pythonfile, float x, float y, float wid, float hei, const std::string & text);
	void CompPython(int room, const std::string & index,const std::string & pythonfile, float x, float y, float wid, float hei, const std::string & text, const std::string & modes);
	void GlobalKeyPython(const std::string & pythonfile);

	void Link (int room, const std::string & index, float x, float y, float wid, float hei, const std::string & text, int to);
	void Launch (int room, const std::string & index, float x, float y, float wid, float hei, const std::string & text);
	void Comp(int room, const std::string & index, float x, float y, float wid, float hei, const std::string & text, const std::string & modes);
	void Python(int room, const std::string & index, float x, float y, float wid, float hei, const std::string & text, const std::string & pythonfile,bool front=false);
	void MessageToRoom(int room, const std::string & text);
	void EnqueueMessageToRoom(int room, const std::string & text);
	void Message(const std::string & text);
	void EnqueueMessage(const std::string & text);
	void RunScript (int room, const std::string & ind, const std::string & pythonfile, float time);
	void TextBox (int room, const std::string & ind, const std::string & text, float x, float y, Vector widheimult, Vector backcol, float backalp, Vector forecol);
	void SetTextBoxText(int room, const std::string & ind, const std::string & text);
    void SetTextBoxFont(int room, const std::string & ind, const std::string & fontname);
    std::string GetTextBoxFont(int room, const std::string & index);
	void SetLinkArea(int room, const std::string & index, float x, float y, float wid, float hei);
	void SetLinkText(int room, const std::string & index, const std::string & text);
	void SetLinkPython(int room, const std::string & index, const std::string & python);
	void SetLinkRoom(int room, const std::string & index, int to);
	void SetLinkEventMask(int room, const std::string & index, const std::string & maskdef); // c=click, u=up, d=down, e=enter, l=leave, m=move
	void EraseLink (int room, const std::string & index);
	void EraseObj (int room, const std::string & index);
	int GetCurRoom ();
	void SetCurRoom (int room);
	int GetNumRoom ();
	bool BuyShip(const std::string & name, bool my_fleet, bool force_base_inventory);
	bool SellShip(const std::string & name);

	// GUI events
	void SetEventData(Dictionary data);
	void SetMouseEventData(const std::string & type, float x, float y, int buttonMask); // [type], [mousex], [mousey], [mousebuttons]
	void SetKeyEventData(const std::string & type, unsigned int keycode, unsigned int modmask=~0);
	void SetKeyStatusEventData(unsigned int modmask=~0);
	const Dictionary& GetEventData();

	// GUI events (engine internals)
	Dictionary& _GetEventData();

	// Auxiliary
	float GetTextHeight(const std::string & text, Vector widheimult);
	float GetTextWidth(const std::string & text, Vector widheimult);
	void LoadBaseInterface(const std::string & name);
	void LoadBaseInterfaceAtDock(const std::string & name, Unit* dockat, Unit *dockee);
	void refreshBaseComputerUI(const class Cargo *dirtyCarg);
	void ExitGame();
}
