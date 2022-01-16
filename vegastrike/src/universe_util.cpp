/// Functions for python modules
///
#include "cmd/script/mission.h"
#include "universe_util.h"
#include "universe.h"
#include "cmd/unit.h"
#include "cmd/unit_interface.h"
#include "cmd/unit_factory.h"	 //for UnitFactory::getMasterPartList()
#include "cmd/collection.h"
#include "star_system.h"
#include <string>
#include "networking/netclient.h"
#include "cmd/music.h"
#include "audiolib.h"
#include "gfx/animation.h"
#include "lin_time.h"
#include "load_mission.h"
#include "config_xml.h"
#include "vs_globals.h"
#include "gfx/particle.h"
#include "cmd/base.h"
#include "options.h"
#include "gldrv/winsys.h"
#include "log.h"

extern vs_options game_options;

extern unsigned int AddAnimation (const QVector & pos, const float size, bool mvolatile, const std::string &name, float percentgrow );
extern void RespawnNow (Cockpit * cp);
extern void TerminateCurrentBase(void);
extern void SetStarSystemLoading (bool value);
extern bool GetStarSystemLoading ();
extern void bootstrap_draw(const std::string &message, Animation * newSplashScreen);
extern Animation* GetSplashScreen();
extern void SetSplashScreen(Animation *ss);
extern const vector <string>& ParseDestinations (const string &value);

using std::string;
								 //less to write
#define activeSys _Universe->activeStarSystem()
void ClientServerSetLightContext (int lightcontext)
{
	GFXSetLightContext(lightcontext);
}


namespace UniverseUtil
{

	void playVictoryTune () {
		muzak->GotoSong(game_options.missionvictorysong);
	}
	int musicAddList(string str) {
		return muzak->Addlist(str.c_str());
	}
	void musicLayerSkip(int layer) {
		muzak->Skip(layer);
	}
	void musicLayerStop(int layer) {
		muzak->Stop(layer);
	}
	void musicLayerPlaySong(string str,int layer) {
		muzak->GotoSong(str,layer);
	}
	void musicLayerPlayList(int which,int layer) {
		if (which!=-1)
			muzak->SkipRandSong(which,layer);
	}
	void musicLayerLoopList (int numloops,int layer) {
		muzak->SetLoops(numloops,layer);
	}
	void musicLayerSetSoftVolume(float vol, float latency_override, int layer) {
		Music::SetVolume(vol,layer,false,latency_override);
	}
	void musicLayerSetHardVolume(float vol, int layer) {
		Music::SetVolume(vol,layer,true);
	}
	void musicSetSoftVolume(float vol, float latency_override) {
		musicLayerSetSoftVolume(vol,latency_override,-1);
	}
	void musicSetHardVolume(float vol) {
		musicLayerSetHardVolume(vol,-1);
	}
	void musicMute (bool stopSound) {
		muzak->Mute(stopSound);
	}
	void playSound(string soundName, QVector loc, Vector speed) {
		int sound = AUDCreateSoundWAV (soundName,false);
		AUDAdjustSound (sound,loc,speed);
		AUDStartPlaying (sound);
		AUDDeleteSound(sound);
	}
  void playSoundCockpit(string soundName) {
    int sound = AUDCreateSoundWAV (soundName,false);
    AUDStartPlaying (sound);
    AUDDeleteSound(sound);
  }
	void StopAllSounds(void) {
		AUDStopAllSounds();
	}
	void cacheAnimation(string aniName) {
		static vector <Animation *> anis;
		anis.push_back (new Animation(aniName.c_str()));
	}
	void playAnimation(string aniName, QVector loc, float size) {
		AddAnimation(loc,size,true,aniName,1);
	}
	void playAnimationGrow(string aniName, QVector loc, float size, float growpercent) {
		AddAnimation(loc,size,true,aniName,growpercent);
	}
	unsigned int getCurrentPlayer() {
		return _Universe->CurrentCockpit();
	}
	int maxMissions () {
		return(game_options.max_missions);
	}
	void addParticle (QVector loc, Vector velocity, Vector color, float size) {
		ParticlePoint p;
		p.loc = loc;
		p.col = color;
		particleTrail.AddParticle (p,velocity,size);
	}

	bool loadGame(const string &savename) {
		Cockpit *cockpit = _Universe->AccessCockpit();
		Unit *player = cockpit->GetParent();
		UniverseUtil::setCurrentSaveGame(savename);
		if (player) {
			if (Network) {
				Network[_Universe->CurrentCockpit()].dieRequest();
			} else {
				player->Kill();
			}
		}
		RespawnNow(cockpit);
		globalWindowManager().shutDown();
		TerminateCurrentBase();
		return true;
	}

	bool saveGame(const string &savename) {
		bool result = true;
		if (Network) {
			Network[_Universe->CurrentCockpit()].saveRequest();
		} else {
			string oldsavename = UniverseUtil::getCurrentSaveGame();
			UniverseUtil::setCurrentSaveGame(savename);
			result = WriteSaveGame(_Universe->AccessCockpit(), false);
			if (result != true) {
				UniverseUtil::setCurrentSaveGame(oldsavename);
			}
		}
		return result;
	}

	void showSplashScreen(const string &filename) {
		static Animation *curSplash = 0;
		if (!filename.empty()) {
			if (curSplash)
				delete curSplash;
			curSplash = new Animation(filename.c_str(),0);
		}
		else if (!curSplash && !GetSplashScreen()) {
			static std::vector<std::string> s = ParseDestinations(game_options.splash_screen);
			int snum=time(NULL)%s.size();
			curSplash = new Animation(s[snum].c_str(),0);
		}
		SetStarSystemLoading(true);
		bootstrap_draw("Loading...",curSplash);
	}

	void showSplashMessage(const string &text) {
		bootstrap_draw(text,0);
	}

	void showSplashProgress(float progress) {
		// Unimplemented
	}

	void hideSplashScreen() {
		SetStarSystemLoading(false);
	}

	bool isSplashScreenShowing() {
		return GetStarSystemLoading();
	}

	void sendCustom(int cp, string cmd, string args, string id) {
		if (cp<0 || cp>=_Universe->numPlayers()) {
			VS_LOG("universe", logvs::NOTICE, "sendCustom %s with invalid player %d", cmd.c_str(), cp);
			return;
		}
		if (Network!=NULL) {
			Network[cp].sendCustom(cmd, args, id);
		} else {
			receivedCustom(cp, true, cmd, args, id);
		}
	}

    #define UNIVERSE_KB_REPEAT_JUST_ONCE
    static std::stack< std::pair<unsigned int,std::pair<int,int> > > s_kb_repeat_stack;
    void enableKeyRepeat() {
        static const bool handle_unicode_kb
            = XMLSupport::parse_bool(vs_config->getVariable("keyboard","enable_unicode","true"));
        int delay_ms = WS_KB_REPEAT_ENABLED_DEFAULT, interval_ms = 0; // WS_KB_REPEAT_INTERVAL
        int delay_bak, interval_bak;
        unsigned int unicode_bak;
        unsigned int unicode = handle_unicode_kb ? WS_UNICODE_FULL : WS_UNICODE_DISABLED;
        
        #ifdef UNIVERSE_KB_REPEAT_JUST_ONCE
        if (s_kb_repeat_stack.size() > 0) {
            winsys_set_kb_mode(unicode, delay_ms, interval_ms, 0, 0, 0);
        } else
        #endif
        {
            winsys_set_kb_mode(unicode, delay_ms, interval_ms, &unicode_bak, &delay_bak, &interval_bak);
            s_kb_repeat_stack.push(std::make_pair(unicode_bak,std::make_pair(delay_bak, interval_bak)));
        }
    }
    bool restoreKeyRepeat() {
        if (s_kb_repeat_stack.size()) {
            std::pair<unsigned int,std::pair<int,int> > bak = s_kb_repeat_stack.top();
            s_kb_repeat_stack.pop();
            winsys_set_kb_mode(bak.first, bak.second.first, bak.second.second, 0, 0, 0);
            return true;
        }
        return false;
    }

}
#undef activeSys
