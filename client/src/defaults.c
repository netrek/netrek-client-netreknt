
/******************************************************************************/
/***  File:  defaults.c                                                     ***/
/***  Function:  This file reads the default parameters from .xtrekrc and   ***/
/***             sets appropriate flags.                                    ***/
/***                                                                        ***/
/***  Author:  Kevin P. Smith 6/11/89                                       ***/
/******************************************************************************/

#include "config.h"
#include "copyright2.h"
#include <stdio.h>
#include "Wlib.h"
#include "defs.h"
#include "struct.h"
#include "data.h"
#include <sys/file.h>
#include "playerlist.h"

#include INC_IO
#include INC_STRINGS
#include INC_LIMITS
#include INC_UNISTD
#include INC_CTYPE

struct stringlist
{
  char   *string;
  char   *value;
  struct stringlist *next;
};

struct stringlist *defaults = NULL;

#define DEFAULTSHIP NUM_TYPES

struct shipdef shipdefaults[NUM_TYPES + 1] =
{
  {"sc", NULL, NULL, NULL, NULL},
  {"dd", NULL, NULL, NULL, NULL},
  {"ca", NULL, NULL, NULL, NULL},
  {"bb", NULL, NULL, NULL, NULL},
  {"as", NULL, NULL, NULL, NULL},
  {"sb", NULL, NULL, NULL, NULL},
  {"ga", NULL, NULL, NULL, NULL},
  {"att", NULL, NULL, NULL, NULL},
  {"default", NULL, NULL, NULL, NULL}
};

int     myshiptype = DEFAULTSHIP;
struct shipdef *myshipdef = &shipdefaults[DEFAULTSHIP];

char   *getenv(const char *);
int     playerlistnum(void);
extern unsigned char getctrlkey(unsigned char **s);

/*************************************************************************/
/***  prob_desc[] will allow us to have simpler code to handle parsing ***/
/***  for these key words.                                             ***/
/*************************************************************************/
static char *prob_desc[] =
{"shld", "dam", "wtmp", "etmp", "arms", "fuel"};
static char *prob_severity[] =
{"low", "mid", "high"};

#ifdef WIN32
int DefaultsLoaded;
char *GetExeDir();

#define XTREKRC "xtrekrc"
#define NETREKRC "netrekrc"

#else

#define XTREKRC ".xtrekrc"
#define NETREKRC ".netrekrc"
#endif /* Win32 */



void initDefaults(char *deffile)
{
  FILE   *fp;
  char    file[256];
// SRS not referenced char   *home;
  char   *v;
  struct stringlist *new;
  struct dmacro_list *dm;
  struct dmacro_list *dm_def;
  int     notdone;
  unsigned char c;
  char   *str;
  
#ifdef MULTILINE_MACROS
  unsigned char keysused[256];

  MZERO(keysused, sizeof(keysused));
#endif

#ifdef WIN32
  DefaultsLoaded = 1;
#endif

#ifdef DEBUG
   printf("Initdefaults\n");
#endif
      
  /* sizeof doesn't work if it isn't in the same source file, shoot me */
  MCOPY(dist_defaults, dist_prefered, sizedist);

  getshipdefaults();

  if (!deffile)
   if (findDefaults(deffile, file))
      deffile = file;
   else
      return;       /* No defaults file! */

  fp = fopen(deffile, "r");
  if (!fp)
    return;
  printf("Reading defaults file %s\n", deffile);

#ifdef NBT
  macrocnt = 0;                                  /* reset macros */
#endif

  STRNCPY(defaultsFile, deffile, sizeof(defaultsFile));
  while (fgets(file, 250, fp))
    {
      if (*file == '#')
        continue;
      if (*file != 0)
        file[strlen(file) - 1] = 0;
      v = file;
      while (*v != ':' && *v != 0)
        {
          v++;
        }
      if (*v == 0)
        continue;
      *v = 0;
      v++;
      while (*v == ' ' || *v == '\t')
        {
          v++;
        }

#ifdef NBT
      /* not very robust but if it breaks nothing will die horribly I think -
       * jmn */
      if (strncmpi(file, "macro.", 6) == 0)
        {
          if (macrocnt == MAX_MACRO)
            {
              fprintf(stderr, "Maximum number of macros is %d\n", MAX_MACRO);
            }
          else
            {
              str = file + 6;
              c = getctrlkey((unsigned char **) &str);
              if (c == '?')
                fprintf(stderr, "Cannot use '?' for a macro\n");
              else
                {
                  macro[macrocnt].type = NBTM;
                  macro[macrocnt].key = c;
                  macro[macrocnt].who = str[1];
                  macro[macrocnt].string = strdup(v);

#ifdef MULTILINE_MACROS
                  if (keysused[macro[macrocnt].key])
                    {
                      macro[keysused[macro[macrocnt].key] - 1].type = NEWMULTIM;
                      macro[macrocnt].type = NEWMULTIM;
                    }
                  else
                    {
                      keysused[macro[macrocnt].key] = macrocnt + 1;
                    }
#endif /* MULTILINE_MACROS */

                  macrocnt++;
                }
            }
        }
      else
#endif

      if (strncmpi(file, "mac.", 4) == 0)
        {
          if (macrocnt == MAX_MACRO)
            {
              fprintf(stderr, "Maximum number of macros is %d\n", MAX_MACRO);
            }
          else
            {
              str = file + 4;
              c = getctrlkey((unsigned char **) &str);
              if (c == '?')
                fprintf(stderr, "Cannot use '?' for a macro\n");
              else
                {
                  macro[macrocnt].key = c;

                  if (str[0] == '.')
                    {
                      if (str[1] == '%')
                        {
                          switch (str[2])
                            {
                            case 'u':
                            case 'U':
                            case 'p':
                              macro[macrocnt].who = MACRO_PLAYER;
                              break;
                            case 't':
                            case 'z':
                            case 'Z':
                              macro[macrocnt].who = MACRO_TEAM;
                              break;
                            case 'g':
                              macro[macrocnt].who = MACRO_FRIEND;
                              break;
                            case 'h':
                              macro[macrocnt].who = MACRO_ENEMY;
                              break;
                            default:
                              macro[macrocnt].who = MACRO_ME;
                              break;
                            }
                          macro[macrocnt].type = NEWMMOUSE;
                        }
                      else
                        {
                          macro[macrocnt].who = str[1];
                          macro[macrocnt].type = NEWMSPEC;
                        }
                    }
                  else
                    {
                      macro[macrocnt].who = '\0';
                      macro[macrocnt].type = NEWM;

#ifdef MULTILINE_MACROS
                      if (keysused[macro[macrocnt].key])
                        {
                          printf("Multiline macros of nonstandard types are not recommended.\n");
                          printf("You might experience strange behaviour of macros.\n");
                          printf("Type: unspecified macro, key: %c.\n", macro[macrocnt].key);
                        }
#endif /* MULTILINE_MACROS */
                    }

#ifdef MULTILINE_MACROS
                  if (keysused[macro[macrocnt].key])
                    {
                      macro[keysused[macro[macrocnt].key] - 1].type = NEWMULTIM;
                      macro[macrocnt].type = NEWMULTIM;
                    }
                  else
                    {
                      keysused[macro[macrocnt].key] = macrocnt + 1;
                    }
#endif /* MULTILINE_MACROS */

                  macro[macrocnt].string = strdup(v);
                  macrocnt++;
                }
            }
        }

      else if (strncmpi(file, "dist.", 5) == 0)
        {
          str = file + 5;
          c = getctrlkey((unsigned char **) &str);
          if (*str != '.')
            {
              str = file + 4;
              c = '\0';
            }
          str++;

          notdone = 1;
          for (dm = &dist_prefered[take], dm_def = &dist_defaults[take];
               dm->name && notdone; dm++, dm_def++)

            {
              if (strcmpi(str, dm->name) == 0)
                {
                  dm->macro = strdup(v);

#ifdef DIST_KEY_NAME
                  if (c)
                    {
                      dm->c = c;
                      dm_def->c = c;
                    }
#endif /* DIST_KEY_NAME */

                  notdone = 0;
                }
            }
        }


#ifdef RCM
      else if (strncmpi(file, "msg.", 4) == 0)
        {
          str = file + 4;
          notdone = 1;

          for (dm = &rcm_msg[0]; dm->name && notdone; dm++)
            {
              if (strcmpi(str, dm->name) == 0)
                {
                  dm->macro = strdup(v);
                  notdone = 0;
                }
            }
        }
#endif /* RCM */

#ifdef TOOLS /* Free configurable macro keys */
      else if (strncmpi(file, "key.", 4) == 0)
        {
          int keycnt;

          if ((keycnt = strlen(keys)) == MAX_KEY-1)
            {
              fprintf(stderr, "Maximum number of keys is %d\n", MAX_KEY-1);
            } else {
              str = file + 4;
              c = getctrlkey((unsigned char **) &str);
              keys[keycnt] = c;
              keys[keycnt+1] = '\0';
              if (*str != '.')
                {
                  c = 't';
              } else {
                str++;
                c = getctrlkey((unsigned char **) &str);
              }
              macroKeys[keycnt].dest = c;
              macroKeys[keycnt].name = strdup(v);
            }
        }
#endif /* Macro Keys */

      else if (strncmpi(file, "singleMacro", 11) == 0)
        {
          int     i;

          str = v;
          for (i = 0; *str; i++)
            singleMacro[i] = getctrlkey((unsigned char **) &str);
          singleMacro[i] = '\0';
        }

      if (*v != 0)
        {
          new = (struct stringlist *) malloc(sizeof(struct stringlist));

          new->next = defaults;
          new->string = strdup(file);
          new->value = strdup(v);
          defaults = new;
        }
    }
  fclose(fp);
}

char   *
        getdefault(char *str)
{
  struct stringlist *sl;

  sl = defaults;
  while (sl != NULL)
    {
      if (strcmpi(sl->string, str) == 0)
        {
          return (sl->value);
        }
      sl = sl->next;
    }
  return (NULL);
}

#ifndef WIN32
/* strcmpi tweaked 9/17/92 E-Mehlhaff to not tweak the strings its' called
 * with... And tweaked again by NBT. Some systems have a demented strdup that
 * doesn't put an end of string at the end and this causes no end of
 * trouble... */
strcmpi(char *str1, char *str2)
{
  char    chr1, chr2;
  register int duh, stop;

  stop = strlen(str1);
  if (stop > strlen(str2))
    return 1;
  else if (stop < strlen(str2))
    return (-1);

  for (duh = 0; duh < stop; duh++)
    {
      chr1 = isupper(str1[duh]) ? str1[duh] : toupper(str1[duh]);
      chr2 = isupper(str2[duh]) ? str2[duh] : toupper(str2[duh]);
      if (chr1 == 0 || chr2 == 0)
        {
          return (0);
        }
      if (chr1 != chr2)
        {
          return (chr2 - chr1);
        }
    }
  return (0);
}
#endif /* Win32 */

/* grr... are you telling me this sort of function isn't in the std libraries
 * somewhere?! sons of satan... - jn */
strncmpi(char *str1, char *str2, int max)
{
  char    chr1, chr2;
  register int duh, stop;

  stop = strlen(str1);

  if (stop < max)
    return -1;

  if (stop > max)
    stop = max;

  if (stop > strlen(str2))
    return 1;

  for (duh = 0; duh < stop; duh++)
    {
      chr1 = isupper(str1[duh]) ? str1[duh] : toupper(str1[duh]);
      chr2 = isupper(str2[duh]) ? str2[duh] : toupper(str2[duh]);
      if (chr1 == 0 || chr2 == 0)
        {
          return (0);
        }
      if (chr1 != chr2)
        {
          return (chr2 - chr1);
        }
    }
  return (0);
}


booleanDefault(char *def, int preferred)
{
  char   *str;

  str = getdefault(def);
  if (str == NULL)
    return (preferred);

  if ((strncmpi(str, "on", 2) == 0) ||
      (strncmpi(str, "true",4) == 0))
    {
      return (1);
    }
  else
    {
      return (0);
    }
}

intDefault(char *def, int preferred)
{
  char   *str;

  str = getdefault(def);
  if (!str)
    return preferred;
  return atoi(str);
}

/* no default file given on command line. See if serverName is defined.  If
 * it exists we look for HOME/.xtrekrc-<serverName> and .xtrekrc-<serverName>
 * Otherwise we try DEFAULT_SERVER. */

/* since this is Find Defaults, I moved all the defaults file checking to *
 * it, and put in support for a system defaults file. * and it uses the
 * access() system call to determine if a defaults *  file exists. * note,
 * access() returns 0 if user can read file, -1 on error or if * they can't. *
 * -EM *
 * 
 * Is anyone else bothered by the fact that this writes to deffile * without
 * really knowing how much of deffile is allocated? *
 * 
 */

int findDefaults(char *deffile, char *file)
{
      
  /* Check base names */
  if (findfile(NETREKRC, file))
     return 1;

  if (findfile(XTREKRC, file))
      return 1;

#ifdef SYSTEM_DEFAULTFILE
  /* now try for a system default defaults file */
  if (findfile(SYSTEM_DEFAULTFILE, file))
      return 1;
#endif

  return 0;
}

void resetdefaults(void)
{
  char   *pek;
  char    tmp[100];
  int     i;

  keepInfo = intDefault("keepInfo", keepInfo);
  showPlanetOwner = booleanDefault("showPlanetOwner", showPlanetOwner);
  newDashboard = intDefault("newDashboard", newDashboard);
  updatespeed = intDefault("updatespersec", updatespeed);
  redrawDelay = intDefault("redrawDelay", redrawDelay);
  logmess = booleanDefault("logging", logmess);

  phaserShrink = intDefault("phaserShrink", phaserShrink);
  if (phaserShrink > 16)
    phaserShrink = 16;

  theirPhaserShrink = intDefault("theirPhaserShrink", theirPhaserShrink);
  if (phaserShrink > 16)
    phaserShrink = 16;
    
  shrinkPhaserOnMiss =
        booleanDefault("shrinkPhaserOnMiss", shrinkPhaserOnMiss);
  
  
#ifdef VSHIELD_BITMAPS
  VShieldBitmaps = booleanDefault("varyShields", VShieldBitmaps);
#endif

  warnShields = booleanDefault("warnShields", warnShields);

#ifdef RSA
  if (RSA_Client >= 0)
    {
      RSA_Client = booleanDefault("useRsa", RSA_Client);
      sprintf(tmp, "useRSA.%s", serverName);
      RSA_Client = booleanDefault(tmp, RSA_Client);

    }
  else
    {
      /* RSA mode was specified in the command line args */
      RSA_Client = (RSA_Client == -2) ? 1 : 0;
    }
#endif


  showLock = intDefault("showLock", showLock);
  if (showLock > 3)
    showLock = 3;

  showPhaser = intDefault("PhaserMsg", showPhaser);
#ifdef XTRA_MESSAGE_UI
  messageHUD = intDefault("messageHUD",messageHUD);
  messHoldThresh = intDefault("messageHoldThresh",messHoldThresh);
#endif
  showPhaser = intDefault("PhaserMsg", showPhaser);
#ifdef PHASER_STATS
    phaserShowStats = booleanDefault("PhaserStats",phaserShowStats);
#endif
  showStats = booleanDefault("showstats", showStats);
  keeppeace = booleanDefault("keeppeace", keeppeace);
  continuetractor = booleanDefault("continuetractor", continuetractor);
  showTractorPressor = booleanDefault("showTractorPressor", showTractorPressor);
  extraBorder = booleanDefault("extraAlertBorder", extraBorder);
  namemode = booleanDefault("showplanetnames", 1);
  reportKills = booleanDefault("reportKills", reportKills);

  udpDebug = intDefault("udpDebug", udpDebug);
  udpClientSend = intDefault("udpClientSend", udpClientSend);
  /* note: requires send */
  udpClientRecv = intDefault("udpClientReceive", udpClientRecv);
  tryUdp = booleanDefault("tryUdp", tryUdp);
  tryUdp1 = tryUdp;
  udpSequenceChk = booleanDefault("udpSequenceCheck", udpSequenceChk);
  baseUdpLocalPort = intDefault("baseUdpLocalPort", baseUdpLocalPort);

#ifdef SHORT_PACKETS
  tryShort = booleanDefault("tryShort", tryShort);
  tryShort1 = tryShort;
#endif

  UseNewDistress = booleanDefault("newDistress", UseNewDistress);
  rejectMacro = booleanDefault("rejectMacro", rejectMacro);
  enemyPhasers = intDefault("enemyPhasers", enemyPhasers);
  pek = getdefault("cloakChars");
  if (pek != (char *) NULL)
    STRNCPY(cloakChars, pek, 3);
  showIND = booleanDefault("showIND", showIND);
  InitPlayerList();

#ifdef IGNORE_SIGNALS_SEGV_BUS
  if (ignore_signals >= 0)
    {
      ignore_signals = booleanDefault("ignoreSignals", ignore_signals);

      if (ignore_signals)
        printf("Ignoring signals SIGSEGV and SIGBUS\n");
    }
  else
    {
      /* ignoresignals mode was specified in the command line args */
      ignore_signals = (ignore_signals == -1) ? 1 : 0;
    }
#endif

highlightFriendlyPhasers = booleanDefault("highlightFriendlyPhasers", 
					  highlightFriendlyPhasers);

#ifdef MOUSE_AS_SHIFT
  mouse_as_shift = booleanDefault("mouseAsShift", mouse_as_shift);
#endif

#ifdef MOTION_MOUSE
  motion_mouse = booleanDefault("continuousMouse", motion_mouse);

  user_motion_thresh = intDefault("motionThresh", user_motion_thresh);
#endif

#ifdef SHIFTED_MOUSE
  extended_mouse = booleanDefault("shiftedMouse", extended_mouse);
#endif

  /* SRS 12/94, ignore the Capslock key */
  ignoreCaps = booleanDefault("ignoreCaps", ignoreCaps);

  showMySpeed = booleanDefault("showMySpeed", showMySpeed);
  
#ifdef SOUND
  sound_init = booleanDefault("sound", sound_init);
#endif

#ifdef TOOLS
  shelltools = booleanDefault("shellTools", shelltools);
#endif


  shipdefaults[DEFAULTSHIP].keymap = (unsigned char *) getdefault("keymap");
  shipdefaults[DEFAULTSHIP].buttonmap = (unsigned char *) getdefault("buttonmap");
  shipdefaults[DEFAULTSHIP].ckeymap = (unsigned char *) getdefault("ckeymap");

  for (i = DEFAULTSHIP; i >= 0; i--)
    {
      STRNCPY(tmp, "rcfile-", 8);
      strcat(tmp, shipdefaults[i].name);
      if (pek = getdefault(tmp))
        shipdefaults[i].rcfile = pek;
      else
        shipdefaults[i].rcfile = shipdefaults[DEFAULTSHIP].rcfile;

      STRNCPY(tmp, "keymap-", 8);
      strcat(tmp, shipdefaults[i].name);
      if (pek = getdefault(tmp))
        shipdefaults[i].keymap = (unsigned char *) pek;
      else
        shipdefaults[i].keymap = shipdefaults[DEFAULTSHIP].keymap;

      STRNCPY(tmp, "ckeymap-", 9);
      strcat(tmp, shipdefaults[i].name);
      if (pek = getdefault(tmp))
        shipdefaults[i].ckeymap = (unsigned char *) pek;
      else
        shipdefaults[i].ckeymap = shipdefaults[DEFAULTSHIP].ckeymap;

      STRNCPY(tmp, "buttonmap-", 11);
      strcat(tmp, shipdefaults[i].name);
      if (pek = getdefault(tmp))
        shipdefaults[i].buttonmap = (unsigned char *) pek;
      else
        shipdefaults[i].buttonmap = shipdefaults[DEFAULTSHIP].buttonmap;
    }
  myshipdef = &shipdefaults[myshiptype];
}

void shipchange(int type)
{
  if (type == myshiptype)
    return;
  myshiptype = type;
  myshipdef = &shipdefaults[type];
  if (shipdefaults[type].rcfile)
    {
      initDefaults(shipdefaults[type].rcfile);
      resetdefaults();
    }
  initkeymap();
}


/* Generally useful function that searches for a file
   in the current and home directories, also
   the executable directory on Win32 */

#ifdef DEBUG
#define CHECK_FILE \
      printf("Checking for file %s...\n", found); \
      accessible = access(found, R_OK); \
      if (accessible == 0)\
        return 1; 
#else
#define CHECK_FILE \
      accessible = access(found, R_OK); \
      if (accessible == 0)\
        return 1; 
#endif

int findfile(char *fname, char *found)
{
   int accessible;
   char *home;

   /* check current directory first */
#ifdef DEBUG
   printf("Checking for file %s\n", fname); 
#endif   
   accessible = access(fname, R_OK); 
   if (accessible == 0)
      {
      strcpy(found, fname);
      return 1;
      }

   /* Check home directory next */
   home = getenv("HOME");
   if (home)
      {
      int len = strlen(home);
      if (home[len-1] == '/'
#ifdef WIN32
          || home[len-1] == '\\'
#endif
          )
         sprintf(found, "%s%s", home, fname);
      else
         sprintf(found, "%s/%s", home, fname);
      }
   CHECK_FILE;
   
#ifdef WIN32
   /* On Windows also check executable directory */
   home = GetExeDir();
   if (home)
      {
      int len = strlen(home);
      if (home[len-1] == '/' || home[len-1] == '\\' )
         sprintf(found, "%s%s", home, fname);
      else
         sprintf(found, "%s/%s", home, fname);
      }
   CHECK_FILE;
#endif /*Win32*/

   return 0;
}
