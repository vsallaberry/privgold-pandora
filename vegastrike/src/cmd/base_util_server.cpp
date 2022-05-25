#include <string>
#include <stdlib.h>
#include "unit_generic.h"
#include "base_util.h"
namespace BaseUtil {
	int Room (const std::string & text) {
            return 0;
	}
	void Texture(int room, const std::string & index, const std::string & file, float x, float y) {
	}
    void Video(int room, const std::string & index, const std::string & file, float x, float y) {
    }
    void VideoStream(int room, const std::string & index, const std::string & file, float x, float y, float w, float h) {
    }
	void Ship (int room, const std::string & index,QVector pos,Vector Q, Vector R) {
	}
	void Link (int room, const std::string & index, float x, float y, float wid, float hei, const std::string & text, int to) {
		LinkPython (room, index, "",x, y,wid, hei, text, to);
	}
	void LinkPython (int room, const std::string & index,const std::string & pythonfile, float x, float y, float wid, float hei, const std::string & text, int to) {
	}
	void Launch (int room, const std::string & index, float x, float y, float wid, float hei, const std::string & text) {
		LaunchPython (room, index,"", x, y, wid, hei, text);
	}
	void LaunchPython (int room, const std::string & index,const std::string & pythonfile, float x, float y, float wid, float hei, const std::string & text) {
	}
	void EjectPython (int room, const std::string & index,const std::string & pythonfile, float x, float y, float wid, float hei, const std::string & text) {
	}
	void Comp(int room, const std::string & index, float x, float y, float wid, float hei, const std::string & text, const std::string & modes) {
	  CompPython(room, index,"", x, y, wid, hei, text,modes) ;
 
	}
	void CompPython(int room, const std::string & index,const std::string & pythonfile, float x, float y, float wid, float hei, const std::string & text, const std::string & modes) {
	}
	void Python(int room, const std::string & index, float x, float y, float wid, float hei, const std::string & text, const std::string & pythonfile) {
	}
	void Message(const std::string & text) {
	}
	void EnqueueMessage(const std::string & text) {
	}
	void EraseLink (int room, const std::string & index) {
	}
	void EraseObj (int room, const std::string & index) {
	}
	int GetCurRoom () {
            return 0;
	}
	int GetNumRoom () {
            return 1;
	}
	void refreshBaseComputerUI(const class Cargo *carg) {
	}
}
