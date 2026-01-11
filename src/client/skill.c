/*
 * Part of Astonia Client (c) Daniel Brockhaus. Please read license.txt.
 *
 * Skills
 *
 * The game's skill list.
 *
 */

#include <stdint.h>

#include "../../src/astonia.h"
#include "../../src/game.h"
#include "../../src/game/_game.h"
#include "../../src/client.h"

int _game_v_profbase=V3_PROFBASE;
int *game_v_profbase=&_game_v_profbase;

int _game_v_max=43+20;
int *game_v_max=&_game_v_max;

struct skill v3_game_skill[V_MAX]={
    //  Bases          Cost W M (0=not raisable, 1=skill, 2=attribute, 3=power)
    // Powers
    {"Hitpoints",-1,-1,-1,3,10},    // 0		done
    {"Endurance",-1,-1,-1,3,10},    // 1
    {"Mana",-1,-1,-1,3,10},    // 2		done

    // Attributes
    {"Wisdom",-1,-1,-1,2,10},    // 3		done
    {"Intuition",-1,-1,-1,2,10},    // 4		done
    {"Agility",-1,-1,-1,2,10},    // 5		done
    {"Strength",-1,-1,-1,2,10},    // 6		done

    // Values
    {"Armor",-1,-1,-1,0,0},    // 7		done
    {"Weapon",-1,-1,-1,0,0},    // 8		done
    {"Light",-1,-1,-1,0,0},    // 9		done
    {"Speed",V3_AGI,V3_AGI,V3_STR,0,0},    // 10		done

    {"Pulse",V3_INT,V3_INT,V3_WIS,1,1},    // 11		done

    // Primary Fighting Skills
    {"Dagger",V3_INT,V3_AGI,V3_STR,1,1},    // 12		done
    {"Hand to Hand",V3_AGI,V3_STR,V3_STR,1,1},    // 13		done
    {"Staff",V3_INT,V3_AGI,V3_STR,1,1},    // 14		done
    {"Sword",V3_INT,V3_AGI,V3_STR,1,1},    // 15		done
    {"Two-Handed",V3_AGI,V3_STR,V3_STR,1,1},    // 16		done

    // Secondary Fighting Skills
    {"Armor Skill",V3_AGI,V3_AGI,V3_STR,1,1},    // 17		done
    {"Attack",V3_INT,V3_AGI,V3_STR,1,1},    // 18		done
    {"Parry",V3_INT,V3_AGI,V3_STR,1,1},    // 19		done
    {"Warcry",V3_INT,V3_AGI,V3_STR,1,1},    // 20
    {"Tactics",V3_INT,V3_AGI,V3_STR,1,1},     // 21
    {"Surround Hit",V3_INT,V3_AGI,V3_STR,1,1},    // 22
    {"Body Control",V3_INT,V3_AGI,V3_STR,1,1},    // 23		done
    {"Speed Skill",V3_INT,V3_AGI,V3_STR,1,1},    // 24		done

    // Misc. Skills
    {"Bartering",V3_INT,V3_INT,V3_WIS,1,1},    // 25
    {"Perception",V3_INT,V3_INT,V3_WIS,1,1},    // 26		done
    {"Stealth",V3_INT,V3_AGI,V3_AGI,1,1},    // 27		done

    // Spells
    {"Bless",V3_INT,V3_INT,V3_WIS,1,1},    // 28		done
    {"Heal",V3_INT,V3_INT,V3_WIS,1,1},    // 29		done
    {"Freeze",V3_INT,V3_INT,V3_WIS,1,1},    // 30		done
    {"Magic Shield",V3_INT,V3_INT,V3_WIS,1,1},    // 31		done
    {"Lightning",V3_INT,V3_INT,V3_WIS,1,1},    // 32		done
    {"Fire",V3_INT,V3_INT,V3_WIS,1,1},    // 33		done
    {"empty",V3_INT,V3_INT,V3_WIS,1,1},    // 34		done

    {"Regenerate",V3_STR,V3_STR,V3_STR,1,1},    // 35		done
    {"Meditate",V3_WIS,V3_WIS,V3_WIS,1,1},    // 36		done
    {"Immunity",V3_INT,V3_WIS,V3_STR,1,1},    // 37

    {"Ancient Knowledge",V3_WIS,V3_INT,V3_STR,1,1},    // 38

    {"Duration",V3_WIS,V3_INT,V3_STR,1,1},    // 39
    {"Rage",V3_WIS,V3_INT,V3_STR,1,1},    // 40
    {"Resist Cold",-1,-1,-1,0,1},    // 41
    {"Profession",-1,-1,-1,3,1},    // 42

    {"Athlete",-1,-1,-1,0,1},    // 0
    {"Alchemist",-1,-1,-1,0,1},    // 1
    {"Miner",-1,-1,-1,0,1},    // 2
    {"Assassin",-1,-1,-1,0,1},    // 3
    {"Thief",-1,-1,-1,0,1},    // 4
    {"Light",-1,-1,-1,0,1},    // 5
    {"Dark",-1,-1,-1,0,1},    // 6
    {"Trader",-1,-1,-1,0,1},    // 7
    {"Mercenary",-1,-1,-1,0,1},    // 8
    {"Clan",-1,-1,-1,0,1},    // 9
    {"Herbalist",-1,-1,-1,0,1},    // 10
    {"empty",-1,-1,-1,0,1},    // 11
    {"empty",-1,-1,-1,0,1},    // 12
    {"empty",-1,-1,-1,0,1},    // 13
    {"empty",-1,-1,-1,0,1},    // 14
    {"empty",-1,-1,-1,0,1},    // 15
    {"empty",-1,-1,-1,0,1},    // 16
    {"empty",-1,-1,-1,0,1},    // 17
    {"empty",-1,-1,-1,0,1},    // 18
    {"empty",-1,-1,-1,0,1} // 19
};

struct skill v35_game_skill[V_MAX] = {
    //  Bases          Cost W M (0=not raisable, 1=skill, 2=attribute, 3=power)
    // Powers
    {"Hitpoints", -1, -1, -1, 3, 25}, // 0		done
    {"Endurance", -1, -1, -1, 3, 25}, // 1		done
    {"Mana", -1, -1, -1, 3, 25}, // 2		done

    // Attributes
    {"Wisdom", -1, -1, -1, 2, 10}, // 3		done
    {"Intuition", -1, -1, -1, 2, 10}, // 4		done
    {"Agility", -1, -1, -1, 2, 10}, // 5		done
    {"Strength", -1, -1, -1, 2, 10}, // 6		done

    // Values
    {"Armor", -1, -1, -1, 0, 0}, // 7		done
    {"Weapon", -1, -1, -1, 0, 0}, // 8		done
    {"Offense", -1, -1, -1, 0, 0}, // 9		done
    {"Defense", -1, -1, -1, 0, 0}, // 10		done
    {"Light", -1, -1, -1, 0, 0}, // 11		done
    {"Speed", V35_AGI, V35_AGI, V35_STR, 0, 0}, // 12		done

    // Primary Fighting Skills
    {"Dagger", V35_INT, V35_AGI, V35_STR, 1, 1}, // 13		done
    {"Hand to Hand", V35_AGI, V35_STR, V35_STR, 1, 1}, // 14		done
    {"Staff", V35_INT, V35_AGI, V35_STR, 1, 1}, // 15		done
    {"Sword", V35_INT, V35_AGI, V35_STR, 1, 1}, // 16		done
    {"Two-Handed", V35_AGI, V35_STR, V35_STR, 1, 1}, // 17		done

    // Secondary Fighting Skills
    {"Attack", V35_INT, V35_AGI, V35_STR, 1, 1}, // 18		done
    {"Parry", V35_INT, V35_AGI, V35_STR, 1, 1}, // 19		done
    {"Warcry", V35_INT, V35_AGI, V35_STR, 1, 1}, // 20		done
    {"Tactics", V35_INT, V35_AGI, V35_STR, 1, 1}, // 21		done
    {"Surround Hit", V35_INT, V35_AGI, V35_STR, 1, 1}, // 22		done
    {"Speed Skill", V35_INT, V35_AGI, V35_STR, 1, 1}, // 23		done

    // Misc. Skills
    {"Bartering", V35_INT, V35_INT, V35_WIS, 1, 1}, // 24		done
    {"Perception", V35_INT, V35_INT, V35_WIS, 1, 1}, // 25		done
    {"Stealth", V35_INT, V35_AGI, V35_AGI, 1, 1}, // 26		done

    // Spells
    {"Bless", V35_INT, V35_INT, V35_WIS, 1, 1}, // 27		done
    {"Heal", V35_INT, V35_INT, V35_WIS, 1, 1}, // 28		done
    {"Freeze", V35_INT, V35_INT, V35_WIS, 1, 1}, // 29		done
    {"Magic Shield", V35_INT, V35_INT, V35_WIS, 1, 1}, // 30		done
    {"Lightning", V35_INT, V35_INT, V35_WIS, 1, 1}, // 31		done
    {"Fire", V35_INT, V35_INT, V35_WIS, 1, 1}, // 32		done

    {"Regenerate", V35_STR, V35_STR, V35_STR, 1, 1}, // 33		done
    {"Meditate", V35_WIS, V35_WIS, V35_WIS, 1, 1}, // 34		done
    {"Immunity", V35_INT, V35_WIS, V35_STR, 1, 1}, // 35		done

    {"Ancient Knowledge", V35_INT, V35_WIS, V35_STR, 0, 1}, // 36		done
    {"Duration", V35_INT, V35_WIS, V35_STR, 1, 1}, // 37		done
    {"Rage", V35_INT, V35_STR, V35_STR, 1, 1}, // 38		done
    {"Resist Cold", -1, -1, -1, 0, 1}, // 39		done
    {"Profession", -1, -1, -1, 3, 1}, // 40

    {"empty", -1, -1, -1, 3, 1}, // 41
    {"empty", -1, -1, -1, 3, 1}, // 42
    {"empty", -1, -1, -1, 3, 1}, // 43
    {"empty", -1, -1, -1, 3, 1}, // 44
    {"empty", -1, -1, -1, 3, 1}, // 45
    {"empty", -1, -1, -1, 3, 1}, // 46
    {"empty", -1, -1, -1, 3, 1}, // 47
    {"empty", -1, -1, -1, 3, 1}, // 48
    {"empty", -1, -1, -1, 3, 1}, // 49


    {"Smith", -1, -1, -1, 0, 1}, // 0
    {"Alchemist", -1, -1, -1, 0, 1}, // 1
    {"Miner", -1, -1, -1, 0, 1}, // 2
    {"Enhancer", -1, -1, -1, 0, 1}, // 3
    {"Mercenary", -1, -1, -1, 0, 1}, // 4
    {"Trader", -1, -1, -1, 0, 1}, // 5
    {"empty", -1, -1, -1, 0, 1}, // 6
    {"empty", -1, -1, -1, 0, 1}, // 7
    {"empty", -1, -1, -1, 0, 1}, // 8
    {"empty", -1, -1, -1, 0, 1} // 9
};

struct skill *game_skill = v3_game_skill;

__declspec(dllexport) int raise_cost(int v,int n) {
    int nr;

    nr=n-game_skill[v].start+1+5;

    if (sv_ver==30 && value[0][V3_ATTACK] && value[0][V3_BLESS]) return max(1,nr*nr*nr*game_skill[v].cost*4/30);
    else return max(1,nr*nr*nr*game_skill[v].cost/10);
}

char *v3_game_skilldesc[]={
    "Hitpoints ('life force') are reduced as you battle and sustain injury. The top red line above your head shows your Hitpoints level. If your Hitpoints level drops to zero, then you will die!",
    "Endurance enables you to run and fight with greater speed. When your Endurance is gone, you will slow down and feel exhausted.",
    "Magic users need Mana to cast spells. If your Mana runs out, then you can no longer cast spells or perform magic. If you stand completely still for a few moments, your Mana will replenish itself.",
    "Wisdom, a base attribute, deepens your comprehension of the secrets of spells and magic; enhances all spells and some skills.",
    "Intuition helps you remain calm and focused when fighting. This base attribute is needed for all spells and most other skills.",
    "Agility will improve your control over your body movements. This base attribute enhances all fighting skills; it will help you move and fight faster.",
    "Strength increases your physical power, allowing you to hit harder. This base attribute is needed for all fighting skills and some other skills.",

    "The amount of protection given by your armor.",
    "The amount of damage your weapon is doing.",
    "The amount of light you are radiating.",
    "How fast your movements are.",

    "Destroys the weakened enemies that surround you by transferring their Hitpoints to you in form of Mana.",

    "The Dagger skill helps you to hit the enemy more often when attacking and increases your chances to parry an enemy's hits; this skill is mainly used when fighting with daggers.",
    "This skill helps you to hit the enemy more often when attacking and increases your chances to parry an enemy's hits; this skill is mainly used when fighting with no weapons.",
    "The Staff skill helps you to hit the enemy more often when attacking and increases your chances to parry an enemy's hits; this skill is mainly used when fighting with a mage's staff.",
    "The Sword skill helps you to hit the enemy more often when attacking and increases your chances to parry an enemy's hits; this skill is mainly used when fighting with swords or other similar weapons.",
    "The Two-Handed skill helps you to hit the enemy more often when attacking and increases your chances to parry an enemy's hits; this skill is mainly used when fighting with Two-Handed weapons.",

    "This skill allows you to wear stronger armor. It may also enhance the protection value of your armor.",
    "Raise this skill to exploit your enemy's weaknesses and hit him when his defenses are down.",
    "Parry allows you to raise your defenses by fending off hits from the enemy.",
    "In the heat of battle, cry out loud, and your enemy will be stunned in terror.",
    "Tactical usage of Attack and Parry; transfers part of Parry to Attack when you hit, and vice versa when you defend.",
    "Inflicts injury to the enemies on your left side and on your right side.",
    "As you train, your body will become hard and strong, providing you with additional protection from enemy hits; Body Control helps increase your weapon value too.",
    "Helps make you a better, faster fighter - the faster you fight, the faster you kill, and the less injury you sustain. It may save thy life.",

    "Bartering increases your negotiating skills, particularly with merchants, enabling you to purchase shop items at a lower price, and sell your own items to shops for a higher price.",
    "Perception increases the depth of your sight, thus allowing you to see enemies better at night. It also allows you to more easily see those who are using Stealth.",
    "Stealth allows you to approach an enemy without being seen.",

    "Bless increases all attributes. It will make you more powerful, but only for a limited time. Casting one Bless spell costs two Mana points.",
    "This spell will heal injuries and recover Hitpoints - the greater the damage suffered, the more Mana that must be used for healing.",
    "Turns the enemy to ice. Casting one Freeze spell costs two Mana points.",

    "This spell creates a Magic Shield to increase the strength of your armor, but becomes weaker with each enemy hit received. It costs one Mana point to raise Magic Shield by two.",
    "This spell will either send a Lightning ball, or create lightning flashes to harm your enemies. One Lightning spell costs three Mana points.",

    "Fire unleashes its energy in an enormous explosion. Cost: three Mana points.",
    "empty",

    "Regenerate helps recover Hitpoints when you are standing motionless; the higher you train this skill, the faster your Hitpoints will replenish.",
    "Meditate helps a magic user recover Mana; the higher you train this skill, the faster your Mana will replenish when standing still.",
    "Reduces the power of spells cast on you by the enemy.",

    "Your knowledge of the ancients, their language and magic.",

    "Makes spells last longer.",
    "As you fight continuously, your Rage builds up to increase Attack and Parry.",

    "Resisting demonic cold.",
    "Raise this skill to be able to learn/improve your professions.",

    "Increases speed, has three times the effect of Speed Skill; also reduces endurance cost of fast mode by up to 66%; all classes, base cost 6, maxes at 30.",
    "Capable of making full moon, equinox, and solstice potions at any time (depending on profession level attained); all classes, base cost 10, maxes at 50.",
    "Increases probability to find silver/gold by 10% per PPS; reduced endurance cost (by PPS*4%); all classes, base cost 4, maxes at 20.",
    "Gets +1 to attack per PPS when attacking an enemy from the side/behind; if attacking an idle enemy from behind, gets +3 to attack per PPS, +5 damage per PPS, and backstab; all classes, base cost 10, maxes at 50.",
    "Gets +2 to stealth for each PPS; will remain invisible even when next to another character when walking or idle; a PK thief can steal from another PK within level limits; all classes, base cost 6, maxes at 30.",
    "Gets +0.5 per PPS to all bases from 6:00 to 18:00; also gets holy vision (ie. can see undead creatures in the dark but not walls/items/living beings); all classes, base cost 6, maxes at 30.",
    "Gets +0.5 per PPS to all bases from 18:00 to 6:00; also gets infravision (ie. can see other living characters in the dark but not walls/items/undead); cannot be learned by someone knowing 'Master of Light'; all classes, base cost 6, maxes at 30.",
    "Bonus (better prices) when buying/selling in shops; all classes, base cost 4, maxes at 20.",
    "PPS*2% more military points from military missions, also gets paid for doing them; all classes, base cost 4, maxes at 20.",
    "+PPS on all base stats in clan-catacombs; all classes, base cost 6, maxes at 30.",
    "Capable of picking a flower/berry/mushroom every 4-12 hours (depending on profession level attained); all classes, base cost 10, maxes at 30.",
    "Demon, non-player prof.",
    "prof13: write me!",
    "prof14: write me!",
    "prof15: write me!",
    "prof16: write me!",
    "prof17: write me!",
    "prof18: write me!",
    "prof19: write me!",
    "prof20: write me!"
};

char *v35_game_skilldesc[] = {
    "Hitpoints ('life force') are reduced as you battle and sustain injury. The top red line above your head shows "
    "your Hitpoints level. If your Hitpoints level drops to zero, then you will die!",
    "Endurance enables you to run and fight with greater speed. When your Endurance is gone, you will slow down and "
    "feel exhausted.",
    "Magic users need Mana to cast spells. If your Mana runs out, then you can no longer cast spells or perform magic. "
    "If you stand completely still for a few moments, your Mana will replenish itself.",
    "Wisdom, a base attribute, deepens your comprehension of the secrets of spells and magic; enhances all spells and "
    "some skills.",
    "Intuition helps you remain calm and focused when fighting. This base attribute is needed for all spells and most "
    "other skills.",
    "Agility will improve your control over your body movements. This base attribute enhances all fighting skills; it "
    "will help you move and fight faster.",
    "Strength increases your physical power, allowing you to hit harder. This base attribute is needed for all "
    "fighting skills and some other skills.",

    "The amount of protection given by your armor.", "The amount of damage your weapon is doing.",
    "Your overall offensive strength. How high your chances are to hit an enemy with your current weapon.",
    "Your overall defensive strength. How high your chances are to evade an enemy's blow.",
    "The amount of light you are radiating.", "How fast your movements are.",

    "The Dagger skill helps you to hit the enemy more often when attacking and increases your chances to parry an "
    "enemy's hits; this skill is mainly used when fighting with daggers.",
    "This skill helps you to hit the enemy more often when attacking and increases your chances to parry an enemy's "
    "hits; this skill is mainly used when fighting with no weapons.",
    "The Staff skill helps you to hit the enemy more often when attacking and increases your chances to parry an "
    "enemy's hits; this skill is mainly used when fighting with a mage's staff.",
    "The Sword skill helps you to hit the enemy more often when attacking and increases your chances to parry an "
    "enemy's hits; this skill is mainly used when fighting with swords or other similar weapons.",
    "The Two-Handed skill helps you to hit the enemy more often when attacking and increases your chances to parry an "
    "enemy's hits; this skill is mainly used when fighting with Two-Handed weapons.",

    "Increases your chances to hit an enemy.",
    "Parry allows you to raise your defenses by fending off hits from the enemy.",
    "In the heat of battle, cry out loud, and your enemy will be stunned in terror.",
    "Tactical usage of Attack and Parry; transfers part of Parry to Attack when you hit, and vice versa when you "
    "defend.",
    "Inflicts injury to the enemies on your left side and on your right side.",
    "Helps make you a better, faster fighter - the faster you fight, the faster you kill, and the less injury you "
    "sustain. It may save thy life.",

    "Bartering increases your negotiating skills, particularly with merchants, enabling you to purchase shop items at "
    "a lower price, and sell your own items to shops for a higher price.",
    "Perception increases the depth of your sight, thus allowing you to see enemies better at night. It also allows "
    "you to more easily see those who are using Stealth.",
    "Stealth allows you to approach an enemy without being seen.",

    "Bless increases all attributes. It will make you more powerful, but only for a limited time.",
    "This spell will heal injuries and recover Hitpoints - the greater the damage suffered, the more Mana that must be "
    "used for healing.",
    "Turns the enemy to ice.",

    "This spell creates a Magic Shield to increase the strength of your armor, but becomes weaker with each enemy hit "
    "received.",
    "This spell will either send a Lightning ball, or create lightning flashes to harm your enemies.",

    "Fire unleashes its energy in an enormous explosion, or creates a firy ring around you.",

    "Regenerate helps recover Hitpoints when you are standing motionless; the higher you train this skill, the faster "
    "your Hitpoints will replenish.",
    "Meditate helps a magic user recover Mana; the higher you train this skill, the faster your Mana will replenish "
    "when standing still.",
    "Reduces the power of spells cast on you by the enemy.",

    "Your knowledge of the ancients, their language and magic.",

    "Makes spells last longer.", "As you fight continuously, your Rage builds up to increase Attack and Parry.",

    "Resisting demonic cold.", "Raise this skill to be able to learn/improve your professions.",

    "empty", "empty", "empty", "empty", "empty", "empty", "empty", "empty", "empty",

    "Repair weapons more efficiently.", "Create better potions.", "Find more silver/gold in mines.",
    "Build up orbs faster.", "Get money from governor for missions; raise faster in rank.",
    "Better prices from merchants.", "empty", "empty", "prof9: write me!", "prof10: write me!"};

char **game_skilldesc = v3_game_skilldesc;

void set_v35_skilltab(void)
{
	game_skill = v35_game_skill;
	game_skilldesc = v35_game_skilldesc;
	_game_v_profbase = V35_PROFBASE;
	_game_v_max = V35_MAX;
}

static vval_t v3_val(svval_t v)
{
	return (vval_t)v;
}

static vval_t v35_val(svval_t v)
{
	switch (v) {
	case V35_HP:
		return V_HP;
	case V35_ENDURANCE:
		return V_ENDURANCE;
	case V35_MANA:
		return V_MANA;
	case V35_WIS:
		return V_WIS;
	case V35_INT:
		return V_INT;
	case V35_AGI:
		return V_AGI;
	case V35_STR:
		return V_STR;
	case V35_ARMOR:
		return V_ARMOR;
	case V35_WEAPON:
		return V_WEAPON;
	case V35_OFFENSE:
		return V_ILLEGAL;
	case V35_DEFENSE:
		return V_ILLEGAL;
	case V35_LIGHT:
		return V_LIGHT;
	case V35_SPEED:
		return V_SPEED;
	case V35_DAGGER:
		return V_DAGGER;
	case V35_HAND:
		return V_HAND;
	case V35_STAFF:
		return V_STAFF;
	case V35_SWORD:
		return V_SWORD;
	case V35_TWOHAND:
		return V_TWOHAND;
	case V35_ATTACK:
		return V_ATTACK;
	case V35_PARRY:
		return V_PARRY;
	case V35_WARCRY:
		return V_WARCRY;
	case V35_TACTICS:
		return V_TACTICS;
	case V35_SURROUND:
		return V_SURROUND;
	case V35_SPEEDSKILL:
		return V_SPEEDSKILL;
	case V35_BARTER:
		return V_BARTER;
	case V35_PERCEPT:
		return V_PERCEPT;
	case V35_STEALTH:
		return V_STEALTH;
	case V35_BLESS:
		return V_BLESS;
	case V35_HEAL:
		return V_HEAL;
	case V35_FREEZE:
		return V_FREEZE;
	case V35_MAGICSHIELD:
		return V_MAGICSHIELD;
	case V35_FLASH:
		return V_FLASH;
	case V35_FIRE:
		return V_FIREBALL;
	case V35_REGENERATE:
		return V_REGENERATE;
	case V35_MEDITATE:
		return V_MEDITATE;
	case V35_IMMUNITY:
		return V_IMMUNITY;
	case V35_DEMON:
		return V_DEMON;
	case V35_DURATION:
		return V_DURATION;
	case V35_RAGE:
		return V_RAGE;
	case V35_COLD:
		return V_COLD;
	case V35_PROFESSION:
		return V_PROFESSION;

	case V35_ILLEGAL:
		return V_ILLEGAL;
	}

	return V_ILLEGAL; // not reached
}

vval_t v_val(svval_t v)
{
	if (sv_ver == 35)  return v35_val(v);
	else return v3_val(v);
}

static svval_t sv3_val(vval_t v)
{
	return (svval_t)v;
}

static svval_t sv35_val(vval_t v)
{
	switch (v) {
	case V_HP:
		return V35_HP;
	case V_ENDURANCE:
		return V35_ENDURANCE;
	case V_MANA:
		return V35_MANA;
	case V_WIS:
		return V35_WIS;
	case V_INT:
		return V35_INT;
	case V_AGI:
		return V35_AGI;
	case V_STR:
		return V35_STR;
	case V_ARMOR:
		return V35_ARMOR;
	case V_WEAPON:
		return V35_WEAPON;
	case V_LIGHT:
		return V35_LIGHT;
	case V_SPEED:
		return V35_SPEED;
	case V_PULSE:
		return V35_ILLEGAL;
	case V_DAGGER:
		return V35_DAGGER;
	case V_HAND:
		return V35_HAND;
	case V_STAFF:
		return V35_STAFF;
	case V_SWORD:
		return V35_SWORD;
	case V_TWOHAND:
		return V35_TWOHAND;
	case V_ARMORSKILL:
		return V35_ILLEGAL;
	case V_ATTACK:
		return V35_ATTACK;
	case V_PARRY:
		return V35_PARRY;
	case V_WARCRY:
		return V35_WARCRY;
	case V_TACTICS:
		return V35_TACTICS;
	case V_SURROUND:
		return V35_SURROUND;
	case V_BODYCONTROL:
		return V35_ILLEGAL;
	case V_SPEEDSKILL:
		return V35_SPEEDSKILL;
	case V_BARTER:
		return V35_BARTER;
	case V_PERCEPT:
		return V35_PERCEPT;
	case V_STEALTH:
		return V35_STEALTH;
	case V_BLESS:
		return V35_BLESS;
	case V_HEAL:
		return V35_HEAL;
	case V_FREEZE:
		return V35_FREEZE;
	case V_MAGICSHIELD:
		return V35_MAGICSHIELD;
	case V_FLASH:
		return V35_FLASH;
	case V_FIREBALL:
		return V35_FIRE;
	case V_REGENERATE:
		return V35_REGENERATE;
	case V_MEDITATE:
		return V35_MEDITATE;
	case V_IMMUNITY:
		return V35_IMMUNITY;
	case V_DEMON:
		return V35_DEMON;
	case V_DURATION:
		return V35_DURATION;
	case V_RAGE:
		return V35_RAGE;
	case V_COLD:
		return V35_COLD;
	case V_PROFESSION:
		return V35_PROFESSION;

	case V_ILLEGAL:
		return V35_ILLEGAL;
	}
	return V35_ILLEGAL; // not reached
}

svval_t sv_val(vval_t v)
{
	if (sv_ver == 35) return sv35_val(v);
	else return sv3_val(v);
}


