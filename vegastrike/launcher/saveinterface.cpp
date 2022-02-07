/* example-start entry entry.c */
#include <string>
#if defined(_WIN32) && _MSC_VER > 1300 
#define __restrict
#endif
#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <string>
#ifdef _WIN32
#include <direct.h>
#include <process.h>
extern void GetRidOfConsole ();
extern void my_sleep (int i);
#else
#include <pwd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#endif

#include "src/common/common.h"
#include "general.h"
#include "launcher.h"

void LoadMissionDialog (char * Filename,int i);
void LoadSaveDialog (char *, char *, int);
void LoadAutoDialog (char *, char *, int);

#define NUM_TITLES 9
static const char * titles [NUM_TITLES] = {"Start Game", "Start New Game","Play Saved Game","Continue Last Game", "Game Settings", "Recover From Autosave","Change Scenario", "Help","Exit Launcher"};
std::string my_mission("explore_universe.mission");
#define NUM_HELPS 7
static const char * helps [NUM_HELPS] = {
  "|START GAME BUTTON|\nStart a new game in the Vegastrike universe with the default settings.\n",
  "|START A NEW GAME BUTTON|\nStart a new game in the Vegastrike universe.\nYou start with a dinged up old llama\nand head from the vega sector with the hope of finding\nprofit and adventure on the frontier.\nTo begin afresh you must choose a new saved game.",
  "|LOAD SAVED GAME BUTTON|\nThis opens up a saved game you had finished playing before.\nTo save you must dock at the base and\nclick on the save/load button and choose the save option.",
  "|CONTINUE GAME|\nUse this button to launch Vegastrike with from a saved\ngame or mission. If you do not choose a mission, you\nwill start in the standard trading/bounty hunting mission.",
  "|GAME SETTINGS BUTTON|\nThis button will start up the configurator to allow you to\nselect your preferred options.",
  "|RECOVER AUTOSAVE BUTTON|\nThis button allows a player to recover their most recently\nplayed game into the selected save game upon next run.\nIf the player quits or the player docks, and then dies,\nit will restore to the last saved position.",
  "|CHANGE SCENERIO BUTTON|\nThis allows you to select which mission vegastrike\nwill start the next time you press one\nof the keys below it. Most missions do not involve\nsave games and will ignore those options,\nhowever the default, in the mission/exploration folder will\nindeed ustilize the save games you specify.\nIf you ignore this option you begin in the standard\ntrading/bounty hunting mission."
};

#if 0 && defined(_WIN32)
std::string ParentDir () {
  static char * final=NULL;
  std::string mypwd;
  if (final==NULL) {
    if (prog_arg!=NULL) {
      // We need to set the path back 2 to make everything ok.
      char *parentdir;
      int pathlen=strlen(prog_arg);
      parentdir=new char[pathlen+1];
      char *c;
      parentdir[pathlen]='\0';
      strncpy ( parentdir, prog_arg, pathlen );
      c = (char*) parentdir;
      
      while (*c != '\0')     /* go to end */
        c++;
      
      while ((*c != '/')&&(*c != '\\')&&(c>parentdir))      /* back up to parent */
        c--;
      if (c>parentdir+1) {
	if (*(c-1)=='.'&&*(c-2)=='/') {
	  c-=2;
	}
      }
      *c = '\0';             /* cut off last part (binary name) */
      
      mypwd = parentdir;
      delete []parentdir;
      final = (char *)malloc(1000);
      final[999]='\0';
      parentdir = new char [1000];
      parentdir[999]='\0';
      getcwd (parentdir,999);
      if (mypwd.length()>0) {
        chdir (mypwd.c_str());
      }
      getcwd (final,999);
      if (strlen(parentdir)>0) {
        chdir (parentdir);
      }
      delete [] parentdir;
    }
  }else {
    mypwd = std::string(final);
  }
  return mypwd;
}
void win32_GoToParentDir () {
  std::string par = ParentDir ();
  //  fprintf (stderr,"changing to %s",par.c_str());
  chdir (par.c_str());
  FILE * fp1 = fopen ("../vegastrike.config","r");
  FILE * fp2 = fopen ("vegastrike.config","r");
  if ((!fp2)&&fp1) {
	  chdir ("..");
  }
}
#endif // _WIN32

int win_close( GtkWidget *w, void *)
{
    return FALSE;
}

GdkWindow * Help (const char *title, const char *text) {
    GtkWidget *window;
    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size(GTK_WINDOW(window), 300,0);
    gtk_window_set_title(GTK_WINDOW(window), title);
    gtk_signal_connect(GTK_OBJECT(window), "delete_event", GTK_SIGNAL_FUNC(win_close), NULL);
    GtkWidget *label = gtk_label_new(text);
    gtk_container_add (GTK_CONTAINER (window), label);
    gtk_widget_show (label);
    gtk_widget_show (window);
    return (GdkWindow*)window;
}

void save_stuff( const char *filename) {
  changehome();
    FILE *file=fopen("save.4.x.txt","wt");
    if (file) {
      fprintf (file, "%s%c", filename,0);
      fclose(file);
    } else {
      Help("Error","ERROR: Unable to open ../save.4.x.txt");
    }
}

int lastSlash (const char * c) {
      int last=0;
      char temp;
      for (unsigned int i=0;(temp=c[i])!='\0';i++) {
        if (temp=='\\'||temp=='/') {
          last = i+1;
        }
      }
      return last;
}
void help_func( GtkWidget *w, int i)
{

#if NUM_HELPS>0
  if (i<0||i>(NUM_HELPS-1)) {
      std::string str=helps[0];
      for (int i=1;i<NUM_HELPS;i++) {
        str+=std::string("\n")+helps[i];
      }
      Help("Help",str.c_str());
    } else {
      Help(titles[i],helps[i]);
    }
#endif

}

#ifdef _WIN32
#include <windows.h>
bool progress=true;
struct stupod {
  char * my_mission;
  char * num;
  stupod (char * a, char *b) {
    my_mission=a;
    num=b;
  }
};
DWORD WINAPI DrawStartupDialog(LPVOID lpParameter) {
	char ** argv = (char **) lpParameter; //stupod *s= (stupod*)lpParameter;
        progress=false;
        Help ("Please wait while vegastrike loads...","Please wait while vegastrike loads...");
		//int pid=spawnl(P_WAIT,vegastrikebin.c_str(),vegastrikebin.c_str(),s->num?s->num:(std::string("\"")+s->my_mission+"\"").c_str(),s->num?(std::string("\"")+s->my_mission+"\"").c_str():NULL,NULL);
        int pid=spawnvp(P_WAIT,vegastrikebin.c_str(),argv);
		if (pid==-1) {
            fprintf(stderr, "cannot launch %s\n", vegastrikebin.c_str());
		}
        /*if (s->num)
          free (s->num);
        free (s->my_mission);
        delete (s);*/
        progress=true;
	return 0;
}
#endif

void launch_mission (bool load_lastsave = false) {
#ifdef _WIN32
  if (!progress)
    return;
#endif
  char ** argv;
  int argc = 1, i_argv = 0, ret = -1;
  // First get the size of argv
  if (load_lastsave) {
      ++argc;
      if (!my_mission.empty()) {
          ++argc;
      }
  }
  int player = my_mission.rfind ("player");
  if (player>0&&player!=std::string::npos) {
    ++argc;
  }
  // Then alloc arv and fill it
  argv = (char **) malloc((argc+1) * sizeof(*argv));
  argv[i_argv++] = strdup(vegastrikebin.c_str());  
  if (player>0&&player!=std::string::npos) {
    char  num [4]={'-','m',(*(my_mission.begin()+(player-1))),'\0'};
    argv[i_argv++] = strdup(num);
  }
  if (load_lastsave) {
      argv[i_argv++] = strdup("-Cgeneral/load_last_savegame=true");
      if (!my_mission.empty()) {
        argv[i_argv++] = strdup(my_mission.c_str());
      }
  }
  argv[i_argv++] = NULL;
  // Display argv
  for(i_argv = 0; i_argv < argc; ++i_argv) {
    fprintf(stderr, "%s ", argv[i_argv]);
  }
  fputc('\n', stderr);
  fflush (stderr);
  changeToData();
#ifndef _WIN32
  if (execvp(vegastrikebin.c_str(), argv) != 0) {
      fprintf(stderr, "ERROR: cannot run %s\n", vegastrikebin.c_str());   
  }
#else
  DWORD id;
  HANDLE hThr=CreateThread(NULL, 0, DrawStartupDialog, (void *) argv, 0, &id);
#endif
}
using std::string;
void file_mission_sel (GtkWidget *w, GtkFileSelection *fs) {
  std::string tmp = gtk_file_selection_get_filename (GTK_FILE_SELECTION (fs));
  FILE * fp =(fopen (tmp.c_str(),"r"));
  if (fp!=NULL) {
    fclose (fp);
	int where=tmp.find ("/mission/");
	if (where==string::npos) {
		where = tmp.find ("\\mission\\");
		if (where==string::npos) {
			where = tmp.find("mission/");
			if (where==string::npos) {
				where=  tmp.find("mission\\");
				if (where==string::npos) {
					where = tmp.find ("mission");
				}
			}
		}
	}
	if (where!=string::npos){
		tmp = tmp.substr(where+strlen("mission/"));
		if (*tmp.begin()=='\\'||*tmp.begin()=='/') {
			tmp = tmp.substr(1);
		}
	}
    my_mission=tmp;
  }
  GdkWindow * ww=gtk_widget_get_parent_window(w);
  gdk_window_destroy(ww);
}
void file_ok_sel( GtkWidget        *w,
                  GtkFileSelection *fs )
{
    if ((gtk_file_selection_get_filename (GTK_FILE_SELECTION (fs)))[0]!='\0') {
      save_stuff(gtk_file_selection_get_filename (GTK_FILE_SELECTION (fs))+lastSlash(gtk_file_selection_get_filename(GTK_FILE_SELECTION(fs))));
    }
    launch_mission(true);
    GdkWindow * ww=gtk_widget_get_parent_window(w);
    gdk_window_destroy(ww);
    //    gtk_main_quit ();

}
void file_ok_auto_sel( GtkWidget        *w,
                  GtkFileSelection *fs )
{
    if ((gtk_file_selection_get_filename (GTK_FILE_SELECTION (fs)))[0]!='\0') {
      char *name=new char [strlen(gtk_file_selection_get_filename (GTK_FILE_SELECTION (fs)))+2];
      strcpy(name+1,gtk_file_selection_get_filename (GTK_FILE_SELECTION (fs))+lastSlash (gtk_file_selection_get_filename (GTK_FILE_SELECTION(fs))));
      name[0]='~';
      save_stuff(name);
    }
    launch_mission(true);
    GdkWindow * ww = gtk_widget_get_parent_window(w);
    gdk_window_destroy(ww);

    //    gtk_main_quit ();
}

void hello( GtkWidget *widget, gpointer   data ) {
    int i=(int)(size_t)data;
    int pid=0;
    switch (i) {
    case 0:
        launch_mission(false);
        break ;
    case 6:
      LoadMissionDialog("Select Mission",i);
      break;
    case 1:
      save_stuff("New_Game");
      launch_mission(true);
      //LoadSaveDialog("New Game","Please type or select the name of the pilot that you wish to create.",i);
      break;
    case 2:
      LoadSaveDialog("Open Game","Please type or select the name of the pilot that you wish to load.",i);
      break;
    case 5:
      LoadAutoDialog("Open Autosave Game","Please type or select the name of the saved game that you wish to autorecover to.",i);
      break;
    case 3:
      launch_mission(true);
      break;
    case 4:
      changeToData();
#ifdef _WIN32
		pid=spawnl(P_NOWAIT,vssetupbin.c_str(), vssetupbin.c_str(),NULL);
		if (pid==-1) {
            fprintf(stderr, "ERROR: cannot run %s\n", vssetupbin.c_str());
		}
#else
        //gdk_window_destroy(gtk_widget_get_parent_window(widget));
        //gtk_main_quit();

        //chdir(origpath.c_str());
        //pid=fork();
		if (1|| pid==0) {
			//if (execlp(vssetupbin.c_str(), vssetupbin.c_str(),NULL) < 0) {
            char ** argv = (char **) malloc((2) * sizeof(*argv));
            argv[0] = strdup(vssetupbin.c_str()); argv[1] = NULL;
            GPid gpid;
            if (gdk_spawn_on_screen(gdk_screen_get_default(), NULL, argv, NULL, (GSpawnFlags)0, NULL, NULL, &gpid, NULL) != TRUE) {
                fprintf(stderr, "ERROR: cannot run %s\n", vssetupbin.c_str());
            }
			return;
		} else if (pid < 0) {
            fprintf(stderr, "ERROR: cannot run %s\n", vssetupbin.c_str());
        }
#endif
      break;
    case 7:
      help_func(0,-1);
      break;
    case 8:
      gtk_main_quit();
      break;
    default:
      fprintf(stderr, "\nERROR...\n");
      gtk_main_quit();
      break;
    }
}

int RunInterface(int * pargc, char *** pargv) {
    //    chdir ("./.vegastrike/save");
    gtk_init (pargc, pargv);
    GtkWidget *window;
    GtkWidget *button;
    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size(GTK_WINDOW(window), 300,350);
    gtk_window_set_title(GTK_WINDOW(window), "Vega Strike Launcher");
    GtkWidget *vbox=gtk_vbox_new(FALSE, 3);
     /* When the window is given the "delete_event" signal (this is given
      * by the window manager, usually by the "close" option, or on the
      * titlebar), we ask it to call the delete_event () function
      * as defined above. The data passed to the callback
      * function is NULL and is ignored in the callback function. */
    gtk_signal_connect(GTK_OBJECT(window), "destroy", GTK_SIGNAL_FUNC(gtk_exit), NULL);
    gtk_signal_connect(GTK_OBJECT(window), "delete_event", GTK_SIGNAL_FUNC(gtk_exit), NULL);
    for (int i=0;i<NUM_TITLES;i++) {
        button = gtk_button_new_with_label (titles[i]);
         
         /* When the button receives the "clicked" signal, it will call the
          * function hello() passing it NULL as its argument.  The hello()
          * function is defined above. */
        gtk_signal_connect (GTK_OBJECT (button), "clicked",
                             GTK_SIGNAL_FUNC (hello), (void*)i);
        gtk_container_add (GTK_CONTAINER (vbox), button);
        gtk_widget_show (button);
    }
    gtk_widget_show (vbox);
    gtk_container_add (GTK_CONTAINER (window), vbox);
         
         /* and the window */
         gtk_widget_show (window);
#if defined(_WIN32)&& (!defined(_WINDOWS)) && !defined(__MINGW32__) && !defined(__CYGWIN__)
	GetRidOfConsole();
#else
	fprintf (stderr, "MISSION = %s\n", my_mission.c_str());
#endif
    gtk_main();
    return(0);
}

#ifdef _WIN32
#include <wchar.h>
#endif
char *makeasc(wchar_t *str) {
	const int WCHAR_SIZE=4;//(sizeof(wchar_t)/sizeof(char));
	int *ptr=(int*)str;
    char * cptr = (char*)str;
	if ((cptr[0]!='\0'&&cptr[1]!='\0')||(cptr[2]!='\0'&&cptr[3]!='\0')) {
		return cptr;
	}
	while (*ptr) {
		*cptr = (char)*ptr;
		if (*cptr=='\0')
			break;
		++cptr;++ptr;
	}
	*cptr='\0';
	return (char*)str;
}

#include "general.h"

void fileop_destroy ( GtkWidget        *w,
			  GtkFileSelection *fs ) {
  if (fs&&fs->fileop_dialog) {
    gtk_widget_destroy(fs->fileop_dialog);
    fs->fileop_dialog=0;
  }
}

void delfile ( GtkWidget        *w,
			  GtkFileSelection *fs ) {
	GtkWidget *i=fs->selection_entry;
	if (i) {
		wchar_t *chr=(wchar_t*)GTK_ENTRY(i)->text;
		char *newstr=makeasc(chr);
		char *remstr=new char[strlen(newstr)+20];
		sprintf(remstr,"../serialized_xml/%s/",newstr);
		glob_t *dirs=FindFiles(remstr,"");
		for (unsigned int i=0;i<dirs->gl_pathc;i++) {
			remove(dirs->gl_pathv[i]);
		}
		rmdir(remstr);
		remove(newstr);
		delete []newstr;
		delete []remstr;
		gtk_file_selection_set_filename (fs,"\0\0\0\0\0\0\0\0");
		gtk_widget_destroy(GTK_FILE_SELECTION(fs)->fileop_dialog);
		GTK_FILE_SELECTION(fs)->fileop_dialog=0;
	}
}

void delfile_conf ( GtkWidget        *w,
		           GtkFileSelection *fs ) {
	GtkWidget *i=fs->selection_entry;
	if (i) {
	    GtkWidget *window;
		wchar_t *chr=(wchar_t*)GTK_ENTRY(i)->text;
		char *newstr=makeasc(chr);
		char *remstr=new char [strlen(newstr)+20];
		sprintf(remstr,"Delete Game \"%s\"?",newstr);
		window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
		gtk_window_set_default_size(GTK_WINDOW(window), 300,0);
		gtk_window_set_title(GTK_WINDOW(window), "Delete Game");
		GtkWidget *vbox=gtk_vbox_new(0,10);
		GtkWidget *cont=gtk_hbox_new(1,5);
		GtkWidget *ok=gtk_button_new_with_label("Delete");
		GtkWidget *cancel=gtk_button_new_with_label("Cancel");
		gtk_signal_connect(GTK_OBJECT(ok), "clicked", GTK_SIGNAL_FUNC(delfile), GTK_OBJECT(fs));
		gtk_signal_connect(GTK_OBJECT(cancel), "clicked", GTK_SIGNAL_FUNC(fileop_destroy), GTK_OBJECT(fs));
		gtk_signal_connect(GTK_OBJECT(window), "delete_event", GTK_SIGNAL_FUNC(win_close), NULL);
		GtkWidget *label = gtk_label_new(remstr);
//		gtk_container_add (GTK_CONTAINER (window), label);
		gtk_box_pack_start(GTK_BOX(vbox),label, TRUE, TRUE, 5);
//		gtk_container_add (GTK_CONTAINER (window), ok);
//		gtk_container_add (GTK_CONTAINER (window), cancel);
		gtk_box_pack_start(GTK_BOX(cont),ok, TRUE, TRUE, 5);
		gtk_box_pack_start(GTK_BOX(cont),cancel, TRUE, TRUE, 5);
		gtk_box_pack_start(GTK_BOX(vbox),cont, TRUE, TRUE, 5);
		gtk_container_add (GTK_CONTAINER (window), vbox);
		gtk_widget_show (label);
		gtk_widget_show (ok);
        GTK_WIDGET_SET_FLAGS(ok, GTK_CAN_DEFAULT);
        GTK_WIDGET_SET_FLAGS(cancel, GTK_CAN_DEFAULT);
        gtk_widget_grab_default(cancel);
		gtk_widget_show (cancel);
		gtk_widget_show (cont);
		gtk_widget_show (vbox);
		gtk_widget_show (window);
		GTK_FILE_SELECTION(fs)->fileop_dialog=window;
	}
}

struct dumbstruct {GtkWidget *filesel, *entrywin;};

void fileop_destroy_dumb ( GtkWidget        *w,
			  dumbstruct *fs ) {
  if (fs->filesel) {
    gtk_widget_destroy(GTK_FILE_SELECTION(fs->filesel)->fileop_dialog);
    GTK_FILE_SELECTION(fs->filesel)->fileop_dialog=0;
  }
  delete fs;
}

void renfile ( GtkWidget        *w,
			  dumbstruct *dmb ) {
	GtkFileSelection *fs=GTK_FILE_SELECTION(dmb->filesel);
	GtkWidget *ent=dmb->entrywin;
	GtkWidget *i=fs->selection_entry;
	if (i && ent) {
		wchar_t *chr=(wchar_t*)GTK_ENTRY(i)->text;
		char *newstr=makeasc(chr);
		chr=(wchar_t*)GTK_ENTRY(ent)->text;
		char *newentstr=makeasc(chr);
		char *remstr=new char[strlen(newstr)+20];
		char *rementstr=new char[strlen(newentstr)+20];
		sprintf(remstr,"../serialized_xml/%s/",newstr);
		sprintf(rementstr,"../serialized_xml/%s/",newentstr);
		rename(remstr,rementstr);
		rename(newstr,newentstr);
		gtk_file_selection_set_filename (fs,"\0\0\0\0\0\0\0\0");
		// "U\0n\0i\0c\0o\0d\0e\0 \0\0\0\0\0"
//		delete []newstr;
		delete []remstr;
//		delete []newentstr;
		delete []rementstr;
	}
	fileop_destroy_dumb(w,dmb);
}

void renfile_conf ( GtkWidget        *w,
		           GtkFileSelection *fs ) {
	GtkWidget *i=fs->selection_entry;
	if (i) {
	    GtkWidget *window;
		wchar_t *chr=(wchar_t*)GTK_ENTRY(i)->text;
		char *newstr=makeasc(chr);
		char *remstr=new char [strlen(newstr)+20];
		sprintf(remstr,"Rename Game \"%s\" to:",newstr);
		window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
		gtk_window_set_default_size(GTK_WINDOW(window), 300,0);
		gtk_window_set_title(GTK_WINDOW(window), "Rename Game");
		GtkWidget *vbox=gtk_vbox_new(0,10);
		GtkWidget *cont=gtk_hbox_new(1,5);
		GtkWidget *ok=gtk_button_new_with_label("Rename");
		GtkWidget *cancel=gtk_button_new_with_label("Cancel");
		GtkWidget *entry=gtk_entry_new();
		dumbstruct *newdum=new dumbstruct ();
		newdum->filesel=GTK_WIDGET(fs);
		newdum->entrywin=entry;
		gtk_signal_connect(GTK_OBJECT(ok), "clicked", GTK_SIGNAL_FUNC(renfile), newdum);
		gtk_signal_connect(GTK_OBJECT(cancel), "clicked", GTK_SIGNAL_FUNC(fileop_destroy_dumb), newdum);
		gtk_signal_connect(GTK_OBJECT(window), "delete_event", GTK_SIGNAL_FUNC(win_close), NULL);
		GtkWidget *label = gtk_label_new(remstr);
//		gtk_container_add (GTK_CONTAINER (window), label);
		gtk_box_pack_start(GTK_BOX(vbox),label, TRUE, TRUE, 5);
		gtk_box_pack_start(GTK_BOX(vbox),entry, TRUE, TRUE, 5);
//		gtk_container_add (GTK_CONTAINER (window), ok);
//		gtk_container_add (GTK_CONTAINER (window), cancel);
		gtk_box_pack_start(GTK_BOX(cont),ok, TRUE, TRUE, 5);
		gtk_box_pack_start(GTK_BOX(cont),cancel, TRUE, TRUE, 5);
		gtk_box_pack_start(GTK_BOX(vbox),cont, TRUE, TRUE, 5);
		gtk_container_add (GTK_CONTAINER (window), vbox);
		gtk_widget_show (label);
		gtk_widget_show (ok);
        GTK_WIDGET_SET_FLAGS(ok, GTK_CAN_DEFAULT);
        GTK_WIDGET_SET_FLAGS(cancel, GTK_CAN_DEFAULT);
        gtk_widget_grab_default(cancel);
		gtk_widget_show (entry);
		gtk_widget_show (cancel);
		gtk_widget_show (cont);
		gtk_widget_show (vbox);
		gtk_widget_show (window);
		GTK_FILE_SELECTION(fs)->fileop_dialog=window;
	}
}

#define BUFFER_SIZE 65530

void copyfp(FILE *f1, FILE *f2) {
	char text [BUFFER_SIZE+1];
	while(!(feof(f1))) {
		int amt_read=fread(text,1,BUFFER_SIZE*sizeof(char),f1);
		fwrite(text,1,amt_read*sizeof(char),f2);
	}
}

void copyfile ( GtkWidget        *w,
			  dumbstruct *dmb ) {
	GtkFileSelection *fs=GTK_FILE_SELECTION(dmb->filesel);
	GtkWidget *ent=dmb->entrywin;
	GtkWidget *i=fs->selection_entry;
	if (i) {
		wchar_t *chr=(wchar_t*)GTK_ENTRY(i)->text;
		char *newstr=makeasc(chr);
		char *remstr=new char[strlen(newstr)+20];
		char *newentstr=makeasc((wchar_t*)GTK_ENTRY(ent)->text);
		char *rementstr=new char [strlen(newentstr)+20];
		sprintf(remstr,"../serialized_xml/%s/",newstr);
		sprintf(rementstr,"../serialized_xml/%s/",newentstr);
#ifdef _WIN32
		mkdir(rementstr);
#else
		mkdir(rementstr,0xffffffff);
#endif
		delete []rementstr;
		FILE *f1=0;
		FILE *f2=0;
		glob_t *dirs=FindFiles(remstr,"");
		for (unsigned int i=0;i<dirs->gl_pathc;i++) {
			f1=fopen(dirs->gl_pathv[i],"rb");
			if (f1) {
				int len=strlen(dirs->gl_pathv[i]);
				char *newchr;
				int j;
				for (j=len-1;j>=0&&dirs->gl_pathv[i][j]!='\\'&&dirs->gl_pathv[i][j]!='/';j--) {
				}
				newchr=dirs->gl_pathv[i]+j+1;
				char *rementstr=new char [strlen(newentstr)+20+strlen(newchr)];
				sprintf(rementstr,"../serialized_xml/%s/%s",newentstr,newchr);
				f2=fopen(rementstr,"w+b");
				if (f2) {
					copyfp(f1,f2);
					fclose(f2);
				}
				fclose(f1);
				delete []rementstr;
			}
		}
		f1=fopen(newstr,"rb");
		if (f1) {
			f2=fopen(newentstr,"wb");
			if (f2) {
				copyfp(f1,f2);
				fclose(f2);
			}
			fclose(f1);
		}
//		delete []newstr;
		delete []remstr;
		gtk_file_selection_set_filename (fs,"\0\0\0\0\0\0\0\0");
//		delete []newentstr;
		gtk_widget_destroy(GTK_FILE_SELECTION(fs)->fileop_dialog);
		GTK_FILE_SELECTION(fs)->fileop_dialog=0;
	}
}

void copyfile_conf ( GtkWidget        *w,
		           GtkFileSelection *fs ) {
	GtkWidget *i=fs->selection_entry;
	if (i) {
	    GtkWidget *window;
		wchar_t *chr=(wchar_t*)GTK_ENTRY(i)->text;
		char *newstr=makeasc(chr);
		char *remstr=new char [strlen(newstr)+20];
		sprintf(remstr,"Copy Game \"%s\" to:",newstr);
		window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
		gtk_window_set_default_size(GTK_WINDOW(window), 300,0);
		gtk_window_set_title(GTK_WINDOW(window), "Copy Game");
		GtkWidget *vbox=gtk_vbox_new(0,10);
		GtkWidget *cont=gtk_hbox_new(1,5);
		GtkWidget *ok=gtk_button_new_with_label("Copy");
		GtkWidget *cancel=gtk_button_new_with_label("Cancel");
		GtkWidget *entry=gtk_entry_new();
		dumbstruct *newdum=new dumbstruct ();
		newdum->filesel=GTK_WIDGET(fs);
		newdum->entrywin=entry;
		gtk_signal_connect(GTK_OBJECT(ok), "clicked", GTK_SIGNAL_FUNC(copyfile), newdum);
		gtk_signal_connect(GTK_OBJECT(cancel), "clicked", GTK_SIGNAL_FUNC(fileop_destroy_dumb), newdum);
		gtk_signal_connect(GTK_OBJECT(window), "delete_event", GTK_SIGNAL_FUNC(win_close), NULL);
		GtkWidget *label = gtk_label_new(remstr);
//		gtk_container_add (GTK_CONTAINER (window), label);
		gtk_box_pack_start(GTK_BOX(vbox),label, TRUE, TRUE, 5);
		gtk_box_pack_start(GTK_BOX(vbox),entry, TRUE, TRUE, 5);
//		gtk_container_add (GTK_CONTAINER (window), ok);
//		gtk_container_add (GTK_CONTAINER (window), cancel);
		gtk_box_pack_start(GTK_BOX(cont),ok, TRUE, TRUE, 5);
		gtk_box_pack_start(GTK_BOX(cont),cancel, TRUE, TRUE, 5);
		gtk_box_pack_start(GTK_BOX(vbox),cont, TRUE, TRUE, 5);
		gtk_container_add (GTK_CONTAINER (window), vbox);
		gtk_widget_show (label);
		gtk_widget_show (ok);
        GTK_WIDGET_SET_FLAGS(ok, GTK_CAN_DEFAULT);
        GTK_WIDGET_SET_FLAGS(cancel, GTK_CAN_DEFAULT);
        gtk_widget_grab_default(cancel);
		gtk_widget_show (entry);
		gtk_widget_show (cancel);
		gtk_widget_show (cont);
		gtk_widget_show (vbox);
		gtk_widget_show (window);
		GTK_FILE_SELECTION(fs)->fileop_dialog=window;
	}
}

void copynormal ( GtkWidget        *w,
			  dumbstruct *dmb ) {
	GtkFileSelection *fs=GTK_FILE_SELECTION(dmb->filesel);
	GtkWidget *ent=dmb->entrywin;
	GtkWidget *i=fs->selection_entry;
	if (i && ent) {
		wchar_t *chr=(wchar_t*)GTK_ENTRY(i)->text;
		char *newstr=makeasc(chr);
		chr=(wchar_t*)GTK_ENTRY(ent)->text;
		char *newentstr=makeasc(chr);
		if (newstr&&newstr[0]!='\0'&&newentstr&&newentstr[0]!='\0') {
			FILE *f1, *f2;
			f1=fopen(newstr,"rb");
			if (f1) {
				f2=fopen(newentstr,"wb");
				if (f2) {
					copyfp(f1,f2);
					fclose(f2);
				}
				fclose(f1);
			}
		}
		gtk_file_selection_set_filename (fs,"\0\0\0\0\0\0\0\0");
//		delete []newstr;
//		delete []newentstr;
	}
	fileop_destroy_dumb(w,dmb);
}

void copynormal_conf ( GtkWidget        *w,
		           GtkFileSelection *fs ) {
	GtkWidget *i=fs->selection_entry;
	if (i) {
	    GtkWidget *window;
		wchar_t *chr=(wchar_t*)GTK_ENTRY(i)->text;
		char *newstr=makeasc(chr);
		char *remstr=new char [strlen(newstr)+20];
		sprintf(remstr,"Copy File \"%s\" to:",newstr);
		window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
		gtk_window_set_default_size(GTK_WINDOW(window), 300,0);
		gtk_window_set_title(GTK_WINDOW(window), "Copy File");
		GtkWidget *vbox=gtk_vbox_new(0,10);
		GtkWidget *cont=gtk_hbox_new(1,5);
		GtkWidget *ok=gtk_button_new_with_label("Copy");
		GtkWidget *cancel=gtk_button_new_with_label("Cancel");
		GtkWidget *entry=gtk_entry_new();
		dumbstruct *newdum=new dumbstruct ();
		newdum->filesel=GTK_WIDGET(fs);
		newdum->entrywin=entry;
		gtk_signal_connect(GTK_OBJECT(ok), "clicked", GTK_SIGNAL_FUNC(copynormal), newdum);
		gtk_signal_connect(GTK_OBJECT(cancel), "clicked", GTK_SIGNAL_FUNC(fileop_destroy_dumb), newdum);
		gtk_signal_connect(GTK_OBJECT(window), "delete_event", GTK_SIGNAL_FUNC(win_close), NULL);
		GtkWidget *label = gtk_label_new(remstr);
//		gtk_container_add (GTK_CONTAINER (window), label);
		gtk_box_pack_start(GTK_BOX(vbox),label, TRUE, TRUE, 5);
		gtk_box_pack_start(GTK_BOX(vbox),entry, TRUE, TRUE, 5);
//		gtk_container_add (GTK_CONTAINER (window), ok);
//		gtk_container_add (GTK_CONTAINER (window), cancel);
		gtk_box_pack_start(GTK_BOX(cont),ok, TRUE, TRUE, 5);
		gtk_box_pack_start(GTK_BOX(cont),cancel, TRUE, TRUE, 5);
		gtk_box_pack_start(GTK_BOX(vbox),cont, TRUE, TRUE, 5);
		gtk_container_add (GTK_CONTAINER (window), vbox);
		gtk_widget_show (label);
		gtk_widget_show (ok);
        GTK_WIDGET_SET_FLAGS(ok, GTK_CAN_DEFAULT);
        GTK_WIDGET_SET_FLAGS(cancel, GTK_CAN_DEFAULT);
        gtk_widget_grab_default(cancel);
		gtk_widget_show (entry);
		gtk_widget_show (cancel);
		gtk_widget_show (cont);
		gtk_widget_show (vbox);
		gtk_widget_show (window);
		GTK_FILE_SELECTION(fs)->fileop_dialog=window;
	}
}

void LoadSaveFunction (char *Filename, char *otherstr, int i, GtkSignalFunc func,const char * default_thing="\0\0\0\0\0\0\0\0",bool usenormalbuttons=false) {
     GtkWidget *filew;
    filew = gtk_file_selection_new (Filename);
	if (!usenormalbuttons){
	gtk_widget_destroy(GTK_FILE_SELECTION(filew)->button_area);
	GTK_FILE_SELECTION(filew)->button_area=gtk_hbox_new(FALSE ,0);
	gtk_widget_show(GTK_FILE_SELECTION(filew)->button_area);
	GtkWidget *newb=gtk_button_new_with_label("Delete Game");
//	char *Addon="\nWarning: Do not use the \"Delete File\" button to delete saved games...\nUse the \"Delete Game\" button instead.";
//	char *otherstr=new char [strlen(othstr)+strlen(Addon)+5];
//	sprintf(otherstr,"%s%s",othstr,Addon);
	gtk_widget_show(newb);
    gtk_container_add(GTK_CONTAINER(GTK_FILE_SELECTION(filew)->button_area),newb);
    gtk_signal_connect (GTK_OBJECT(newb), "clicked", (GtkSignalFunc) delfile_conf, filew );
	newb=gtk_button_new_with_label("Rename Game");
	gtk_widget_show(newb);
    gtk_container_add(GTK_CONTAINER(GTK_FILE_SELECTION(filew)->button_area),newb);
    gtk_signal_connect (GTK_OBJECT(newb), "clicked", (GtkSignalFunc) renfile_conf, filew );
	newb=gtk_button_new_with_label("Copy Game");
	gtk_widget_show(newb);
    gtk_container_add(GTK_CONTAINER(GTK_FILE_SELECTION(filew)->button_area),newb);
    gtk_signal_connect (GTK_OBJECT(newb), "clicked", (GtkSignalFunc) copyfile_conf, filew );
	}else{
		GtkWidget *newb=gtk_button_new_with_label("Copy File");
		gtk_widget_show(newb);
		gtk_container_add(GTK_CONTAINER(GTK_FILE_SELECTION(filew)->button_area),newb);
		gtk_signal_connect (GTK_OBJECT(newb), "clicked", (GtkSignalFunc) copynormal_conf, filew );
	}
    gtk_signal_connect (GTK_OBJECT (GTK_FILE_SELECTION (filew)->ok_button),
                        "clicked", (GtkSignalFunc) func, filew );
    GTK_FILE_SELECTION(filew)->help_button=gtk_button_new_with_label ("Help");
    GTK_WIDGET_SET_FLAGS(GTK_FILE_SELECTION(filew)->help_button, GTK_CAN_DEFAULT);
    gtk_signal_connect (GTK_OBJECT (GTK_FILE_SELECTION (filew)->help_button),
                        "clicked", (GtkSignalFunc) help_func, (void*)i );

    gtk_signal_connect_object (GTK_OBJECT (GTK_FILE_SELECTION
                                            (filew)->cancel_button),
                               "clicked", (GtkSignalFunc) gtk_widget_destroy,
                               GTK_OBJECT (filew));
//    if (default_thing[0]!='\0') {
//      gtk_file_selection_complete (GTK_FILE_SELECTION(filew), default_thing);
//    }else {
      gtk_file_selection_set_filename (GTK_FILE_SELECTION(filew), default_thing);
//    }
    GtkWidget *lbl=gtk_label_new(otherstr);
    gtk_container_add(GTK_CONTAINER(GTK_FILE_SELECTION(filew)->action_area),lbl);
    gtk_widget_show(lbl);
    GtkWidget *box=gtk_label_new("");
    gtk_container_add (GTK_CONTAINER (GTK_FILE_SELECTION(filew)->button_area),box);
    gtk_box_pack_end (GTK_BOX (GTK_FILE_SELECTION(filew)->ok_button->parent),GTK_FILE_SELECTION (filew)->help_button, FALSE, TRUE, 40);
	if (!usenormalbuttons)
		gtk_box_pack_end(GTK_BOX(GTK_FILE_SELECTION(filew)->main_vbox),GTK_FILE_SELECTION(filew)->button_area,FALSE,TRUE,0);
    gtk_widget_show(box);
    gtk_widget_show(GTK_FILE_SELECTION(filew)->help_button);
    gtk_widget_show(filew);
//	delete []otherstr;
}

void LoadMissionDialog (char * Filename,int i) {
  changeToData();
  chdir ("mission");
  char mypwd [1000];
  getcwd (mypwd,1000);
  //  fprintf (stderr,mypwd);
  LoadSaveFunction (Filename,"Select the mission, then run by clicking new or load game.",i,(GtkSignalFunc) file_mission_sel,my_mission.c_str(),true);
}

void LoadSaveDialog (char *Filename,char *otherstr, int i) {
  changehome();
  chdir ("save");
  LoadSaveFunction (Filename,otherstr,i,(GtkSignalFunc) file_ok_sel);
}
void LoadAutoDialog (char *Filename,char *otherstr, int i) {
  changehome();
  chdir ("save");
  LoadSaveFunction (Filename,otherstr,i,(GtkSignalFunc)file_ok_auto_sel);
}
/* example-end */


