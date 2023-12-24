/*
 * Example of modding the ranknames (using vanilla data for easy editing)
 */

DLL_EXPORT char *game_rankname[]={
    "nobody",               //0
    "Private",              //1
    "Private First Class",  //2
    "Lance Corporal",       //3
    "Corporal",             //4
    "Sergeant",             //5     lvl 30
    "Staff Sergeant",       //6
    "Master Sergeant",      //7
    "First Sergeant",       //8     lvl 45
    "Sergeant Major",       //9
    "Second Lieutenant",    //10    lvl 55
    "First Lieutenant",     //11
    "Captain",              //12
    "Major",                //13
    "Lieutenant Colonel",   //14
    "Colonel",              //15
    "Brigadier General",    //16
    "Major General",        //17
    "Lieutenant General",   //18
    "General",              //19
    "Field Marshal",        //20    lvl 105
    "Knight of Astonia",    //21
    "Baron of Astonia",     //22
    "Earl of Astonia",      //23
    "Warlord of Astonia"    //24    lvl 125
};

DLL_EXPORT int game_rankcount=ARRAYSIZE(game_rankname);

