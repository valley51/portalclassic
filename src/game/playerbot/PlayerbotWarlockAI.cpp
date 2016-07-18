
#include "PlayerbotWarlockAI.h"

class PlayerbotAI;
PlayerbotWarlockAI::PlayerbotWarlockAI(Player* const master, Player* const bot, PlayerbotAI* const ai) : PlayerbotClassAI(master, bot, ai)
{
    // DESTRUCTION
    SHADOW_BOLT           = m_ai->initSpell(SHADOW_BOLT_1);
    IMMOLATE              = m_ai->initSpell(IMMOLATE_1);
    SEARING_PAIN          = m_ai->initSpell(SEARING_PAIN_1);
    CONFLAGRATE           = m_ai->initSpell(CONFLAGRATE_1);
    HELLFIRE              = m_ai->initSpell(HELLFIRE_1);
    RAIN_OF_FIRE          = m_ai->initSpell(RAIN_OF_FIRE_1);
    SOUL_FIRE             = m_ai->initSpell(SOUL_FIRE_1); // soul shard spells
    SHADOWBURN            = m_ai->initSpell(SHADOWBURN_1);
    // CURSE
    CURSE_OF_WEAKNESS     = m_ai->initSpell(CURSE_OF_WEAKNESS_1);
    CURSE_OF_THE_ELEMENTS = m_ai->initSpell(CURSE_OF_THE_ELEMENTS_1);
    CURSE_OF_AGONY        = m_ai->initSpell(CURSE_OF_AGONY_1);
    CURSE_OF_EXHAUSTION   = m_ai->initSpell(CURSE_OF_EXHAUSTION_1);
    CURSE_OF_RECKLESSNESS = m_ai->initSpell(CURSE_OF_RECKLESSNESS_1);
    CURSE_OF_SHADOW       = m_ai->initSpell(CURSE_OF_SHADOW_1);
    CURSE_OF_TONGUES      = m_ai->initSpell(CURSE_OF_TONGUES_1);
    CURSE_OF_DOOM         = m_ai->initSpell(CURSE_OF_DOOM_1);
    // AFFLICTION
    AMPLIFY_CURSE         = m_ai->initSpell(AMPLIFY_CURSE_1);
    CORRUPTION            = m_ai->initSpell(CORRUPTION_1);
    DRAIN_SOUL            = m_ai->initSpell(DRAIN_SOUL_1);
    DRAIN_LIFE            = m_ai->initSpell(DRAIN_LIFE_1);
    DRAIN_MANA            = m_ai->initSpell(DRAIN_MANA_1);
    LIFE_TAP              = m_ai->initSpell(LIFE_TAP_1);
    DARK_PACT             = m_ai->initSpell(DARK_PACT_1);
    HOWL_OF_TERROR        = m_ai->initSpell(HOWL_OF_TERROR_1);
    FEAR                  = m_ai->initSpell(FEAR_1);
    SIPHON_LIFE           = m_ai->initSpell(SIPHON_LIFE_1);
    // DEMONOLOGY
    BANISH                = m_ai->initSpell(BANISH_1);
    ENSLAVE_DEMON         = m_ai->initSpell(ENSLAVE_DEMON_1);
    DEMON_SKIN            = m_ai->initSpell(DEMON_SKIN_1);
    DEMON_ARMOR           = m_ai->initSpell(DEMON_ARMOR_1);
    SHADOW_WARD           = m_ai->initSpell(SHADOW_WARD_1);
    SOUL_LINK             = m_ai->initSpell(SOUL_LINK_1);
    SOUL_LINK_AURA        = 25228; // dummy aura applied, after spell SOUL_LINK
    HEALTH_FUNNEL         = m_ai->initSpell(HEALTH_FUNNEL_1);
    DETECT_INVISIBILITY   = m_ai->initSpell(DETECT_INVISIBILITY_1);
    CREATE_FIRESTONE      = m_ai->initSpell(CREATE_FIRESTONE_1);
    CREATE_HEALTHSTONE    = m_ai->initSpell(CREATE_HEALTHSTONE_1);
    CREATE_SOULSTONE      = m_ai->initSpell(CREATE_SOULSTONE_1);
    CREATE_SPELLSTONE     = m_ai->initSpell(CREATE_SPELLSTONE_1);
    // demon summon
    SUMMON_IMP            = m_ai->initSpell(SUMMON_IMP_1);
    SUMMON_VOIDWALKER     = m_ai->initSpell(SUMMON_VOIDWALKER_1);
    SUMMON_SUCCUBUS       = m_ai->initSpell(SUMMON_SUCCUBUS_1);
    SUMMON_FELHUNTER      = m_ai->initSpell(SUMMON_FELHUNTER_1);
    // demon skills should be initialized on demons
    BLOOD_PACT            = 0; // imp skill
    CONSUME_SHADOWS       = 0; // voidwalker skill
    // RANGED COMBAT
    SHOOT                 = m_ai->initSpell(SHOOT_3);

    RECENTLY_BANDAGED     = 11196; // first aid check

    // racial
    ESCAPE_ARTIST         = m_ai->initSpell(ESCAPE_ARTIST_ALL); // gnome
    PERCEPTION            = m_ai->initSpell(PERCEPTION_ALL); // human
    BLOOD_FURY            = m_ai->initSpell(BLOOD_FURY_ALL); // orc
    WILL_OF_THE_FORSAKEN  = m_ai->initSpell(WILL_OF_THE_FORSAKEN_ALL); // undead

    m_lastDemon           = 0;
    m_isTempImp           = false;
    m_CurrentCurse        = 0;
}

PlayerbotWarlockAI::~PlayerbotWarlockAI() {}

CombatManeuverReturns PlayerbotWarlockAI::DoFirstCombatManeuver(Unit* pTarget)
{
    // There are NPCs in BGs and Open World PvP, so don't filter this on PvP scenarios (of course if PvP targets anyone but tank, all bets are off anyway)
    // Wait until the tank says so, until any non-tank gains aggro or X seconds - whichever is shortest
    if (m_ai->GetCombatOrder() & PlayerbotAI::ORDERS_TEMP_WAIT_TANKAGGRO)
    {
        if (m_WaitUntil > m_ai->CurrentTime() && m_ai->GroupTankHoldsAggro())
        {
            return RETURN_NO_ACTION_OK; // wait it out
        }
        else
        {
            m_ai->ClearGroupCombatOrder(PlayerbotAI::ORDERS_TEMP_WAIT_TANKAGGRO);
        }
    }

    if (m_ai->GetCombatOrder() & PlayerbotAI::ORDERS_TEMP_WAIT_OOC)
    {
        if (m_WaitUntil > m_ai->CurrentTime() && !m_ai->IsGroupInCombat())
            return RETURN_NO_ACTION_OK; // wait it out
        else
            m_ai->ClearGroupCombatOrder(PlayerbotAI::ORDERS_TEMP_WAIT_OOC);
    }

    switch (m_ai->GetScenarioType())
    {
        case PlayerbotAI::SCENARIO_PVP_DUEL:
        case PlayerbotAI::SCENARIO_PVP_BG:
        case PlayerbotAI::SCENARIO_PVP_ARENA:
        case PlayerbotAI::SCENARIO_PVP_OPENWORLD:
            return DoFirstCombatManeuverPVP(pTarget);
        case PlayerbotAI::SCENARIO_PVE:
        case PlayerbotAI::SCENARIO_PVE_ELITE:
        case PlayerbotAI::SCENARIO_PVE_RAID:
        default:
            return DoFirstCombatManeuverPVE(pTarget);
            break;
    }

    return RETURN_NO_ACTION_ERROR;
}

CombatManeuverReturns PlayerbotWarlockAI::DoFirstCombatManeuverPVE(Unit* /*pTarget*/)
{
    m_CurrentCurse = 0;
    return RETURN_NO_ACTION_OK;
}

CombatManeuverReturns PlayerbotWarlockAI::DoFirstCombatManeuverPVP(Unit* /*pTarget*/)
{
    return RETURN_NO_ACTION_OK;
}

CombatManeuverReturns PlayerbotWarlockAI::DoNextCombatManeuver(Unit *pTarget)
{
    // Face enemy, make sure bot is attacking
    m_ai->FaceTarget(pTarget);

    switch (m_ai->GetScenarioType())
    {
        case PlayerbotAI::SCENARIO_PVP_DUEL:
        case PlayerbotAI::SCENARIO_PVP_BG:
        case PlayerbotAI::SCENARIO_PVP_ARENA:
        case PlayerbotAI::SCENARIO_PVP_OPENWORLD:
            return DoNextCombatManeuverPVP(pTarget);
        case PlayerbotAI::SCENARIO_PVE:
        case PlayerbotAI::SCENARIO_PVE_ELITE:
        case PlayerbotAI::SCENARIO_PVE_RAID:
        default:
            return DoNextCombatManeuverPVE(pTarget);
            break;
    }

    return RETURN_NO_ACTION_ERROR;
}

CombatManeuverReturns PlayerbotWarlockAI::DoNextCombatManeuverPVE(Unit *pTarget)
{
    if (!m_ai)  return RETURN_NO_ACTION_ERROR;
    if (!m_bot) return RETURN_NO_ACTION_ERROR;

    //Unit* pVictim = pTarget->getVictim();
    bool meleeReach = m_bot->CanReachWithMeleeAttack(pTarget);
    Pet *pet = m_bot->GetPet();
    uint32 spec = m_bot->GetSpec();
    uint8 shardCount = m_bot->GetItemCount(SOUL_SHARD, false, nullptr);

    // Voidwalker is near death - sacrifice it for a shield
    if (pet && pet->GetEntry() == DEMON_VOIDWALKER && SACRIFICE && !m_bot->HasAura(SACRIFICE) && pet->GetHealthPercent() < 10)
        m_ai->CastPetSpell(SACRIFICE);

    // Use healthstone
    if (m_ai->GetHealthPercent() < 30)
    {
        Item* healthStone = m_ai->FindConsumable(HEALTHSTONE_DISPLAYID);
        if (healthStone)
            m_ai->UseItem(healthStone);
    }

    // Voidwalker sacrifice gives shield - but you lose the pet (and it's DPS/tank) - use only as last resort for your own health!
    if (m_ai->GetHealthPercent() < 20 && pet && pet->GetEntry() == DEMON_VOIDWALKER && SACRIFICE && !m_bot->HasAura(SACRIFICE))
        m_ai->CastPetSpell(SACRIFICE);

    if (m_ai->GetCombatStyle() != PlayerbotAI::COMBAT_RANGED && !meleeReach)
        m_ai->SetCombatStyle(PlayerbotAI::COMBAT_RANGED);
    // if in melee range OR can't shoot OR have no ranged (wand) equipped
    else if(m_ai->GetCombatStyle() != PlayerbotAI::COMBAT_MELEE && (meleeReach || SHOOT == 0 || !m_bot->GetWeaponForAttack(RANGED_ATTACK, true, true)))
        m_ai->SetCombatStyle(PlayerbotAI::COMBAT_MELEE);

    //Used to determine if this bot is highest on threat
    Unit *newTarget = m_ai->FindAttacker((PlayerbotAI::ATTACKERINFOTYPE) (PlayerbotAI::AIT_VICTIMSELF | PlayerbotAI::AIT_HIGHESTTHREAT), m_bot);
    if (newTarget) // TODO: && party has a tank
    {
        // Have threat, can't quickly lower it. 3 options remain: Stop attacking, lowlevel damage (wand), keep on keeping on.
        if (newTarget->GetHealthPercent() > 25)
        {
            // If elite
            if (m_ai->IsElite(newTarget))
            {
                // let warlock pet handle it to win some time
                Creature * pCreature = (Creature*) newTarget;
                if (pet)
                {
                    switch (pet->GetEntry())
                    {
                        // taunt the elite and tank it
                        case DEMON_VOIDWALKER:
                            if (TORMENT && m_ai->CastPetSpell(TORMENT, newTarget))
                                return RETURN_NO_ACTION_OK;
                        // maybe give it some love?
                        case DEMON_SUCCUBUS:
                            if (pCreature && pCreature->GetCreatureInfo()->CreatureType == CREATURE_TYPE_HUMANOID)
                                if (SEDUCTION && !newTarget->HasAura(SEDUCTION) && m_ai->CastPetSpell(SEDUCTION, newTarget))
                                    return RETURN_NO_ACTION_OK;
                    }

                }
                // if aggroed mob is a demon or an elemental: banish it
                if (pCreature && (pCreature->GetCreatureInfo()->CreatureType == CREATURE_TYPE_DEMON || pCreature->GetCreatureInfo()->CreatureType == CREATURE_TYPE_ELEMENTAL))
                {
                    if (BANISH && !newTarget->HasAura(BANISH) && CastSpell(BANISH, newTarget))
                        return RETURN_CONTINUE;
                }

                return RETURN_NO_ACTION_OK; // do nothing and pray tank gets aggro off you
            }

            // Not an elite. You could insert FEAR here but in any PvE situation that's 90-95% likely
            // to worsen the situation for the group. ... So please don't.
            return CastSpell(SHOOT, pTarget);
        }
    }

    // Create soul shard
    uint8 freeSpace = m_ai->GetFreeBagSpace();
    uint8 HPThreshold = (m_ai->IsElite(pTarget) ? 10 : 25);
    if (DRAIN_SOUL && pTarget->GetHealthPercent() < HPThreshold && m_ai->In_Reach(pTarget, DRAIN_SOUL) &&
        !pTarget->HasAura(DRAIN_SOUL) && (shardCount < MAX_SHARD_COUNT && freeSpace > 0) && CastSpell(DRAIN_SOUL, pTarget))
    {
        m_ai->SetIgnoreUpdateTime(15);
        return RETURN_CONTINUE;
    }

    if (pet && DARK_PACT && (100 * pet->GetPower(POWER_MANA) / pet->GetMaxPower(POWER_MANA)) > 10 && m_ai->GetManaPercent() <= 20)
        if (m_ai->CastSpell(DARK_PACT, *m_bot))
            return RETURN_CONTINUE;

    // Mana check and replenishment
    if (LIFE_TAP && m_ai->GetManaPercent() <= 20 && m_ai->GetHealthPercent() > 50)
        if (m_ai->CastSpell(LIFE_TAP, *m_bot))
            return RETURN_CONTINUE;

    // HP, mana and aggro checks done
    // Curse the target
    if (CheckCurse(pTarget))
        return RETURN_CONTINUE;

    // Damage Spells
    switch (spec)
    {
        case WARLOCK_SPEC_AFFLICTION:
            if (CORRUPTION && m_ai->In_Reach(pTarget,CORRUPTION) && !pTarget->HasAura(CORRUPTION) && CastSpell(CORRUPTION, pTarget))
                return RETURN_CONTINUE;
            if (IMMOLATE && m_ai->In_Reach(pTarget,IMMOLATE) && !pTarget->HasAura(IMMOLATE) && CastSpell(IMMOLATE, pTarget))
                return RETURN_CONTINUE;
            if (SIPHON_LIFE > 0 && m_ai->In_Reach(pTarget,SIPHON_LIFE) && !pTarget->HasAura(SIPHON_LIFE) && CastSpell(SIPHON_LIFE, pTarget))
                return RETURN_CONTINUE;
            if (SHADOW_BOLT && m_ai->In_Reach(pTarget,SHADOW_BOLT) && CastSpell(SHADOW_BOLT, pTarget))
                return RETURN_CONTINUE;

            return RETURN_NO_ACTION_OK;

        case WARLOCK_SPEC_DEMONOLOGY:
            if (CORRUPTION && m_ai->In_Reach(pTarget,CORRUPTION) && !pTarget->HasAura(CORRUPTION) && CastSpell(CORRUPTION, pTarget))
                return RETURN_CONTINUE;
            if (IMMOLATE && m_ai->In_Reach(pTarget,IMMOLATE) && !pTarget->HasAura(IMMOLATE) && CastSpell(IMMOLATE, pTarget))
                return RETURN_CONTINUE;
            if (SHADOW_BOLT && m_ai->In_Reach(pTarget,SHADOW_BOLT) && CastSpell(SHADOW_BOLT, pTarget))
                return RETURN_CONTINUE;

            return RETURN_NO_ACTION_OK;

        case WARLOCK_SPEC_DESTRUCTION:
            if (SHADOWBURN && pTarget->GetHealthPercent() < (HPThreshold / 2.0) && m_ai->In_Reach(pTarget, SHADOWBURN) && !pTarget->HasAura(SHADOWBURN) && CastSpell(SHADOWBURN, pTarget))
                return RETURN_CONTINUE;
            if (CORRUPTION && m_ai->In_Reach(pTarget,CORRUPTION) && !pTarget->HasAura(CORRUPTION) && CastSpell(CORRUPTION, pTarget))
                return RETURN_CONTINUE;
            if (IMMOLATE && m_ai->In_Reach(pTarget,IMMOLATE) && !pTarget->HasAura(IMMOLATE) && CastSpell(IMMOLATE, pTarget))
                return RETURN_CONTINUE;
            if (CONFLAGRATE && m_ai->In_Reach(pTarget,CONFLAGRATE) && pTarget->HasAura(IMMOLATE) && !m_bot->HasSpellCooldown(CONFLAGRATE) && CastSpell(CONFLAGRATE, pTarget))
                return RETURN_CONTINUE;
            if (SHADOW_BOLT && m_ai->In_Reach(pTarget,SHADOW_BOLT) && CastSpell(SHADOW_BOLT, pTarget))
                return RETURN_CONTINUE;

            return RETURN_NO_ACTION_OK;

            //if (DRAIN_LIFE && LastSpellAffliction < 4 && !pTarget->HasAura(DRAIN_SOUL) && !pTarget->HasAura(DRAIN_LIFE) && !pTarget->HasAura(DRAIN_MANA) && m_ai->GetHealthPercent() <= 70)
            //    m_ai->CastSpell(DRAIN_LIFE, *pTarget);
            //    //m_ai->SetIgnoreUpdateTime(5);
            //else if (HOWL_OF_TERROR && !pTarget->HasAura(HOWL_OF_TERROR) && m_ai->GetAttackerCount() > 3 && LastSpellAffliction < 8)
            //    m_ai->CastSpell(HOWL_OF_TERROR, *pTarget);
            //    m_ai->TellMaster("casting howl of terror!");
            //else if (FEAR && !pTarget->HasAura(FEAR) && pVictim == m_bot && m_ai->GetAttackerCount() >= 2 && LastSpellAffliction < 9)
            //    m_ai->CastSpell(FEAR, *pTarget);
            //    //m_ai->TellMaster("casting fear!");
            //    //m_ai->SetIgnoreUpdateTime(1.5);
            //else if (RAIN_OF_FIRE && LastSpellDestruction < 3 && m_ai->GetAttackerCount() >= 3)
            //    m_ai->CastSpell(RAIN_OF_FIRE, *pTarget);
            //    //m_ai->TellMaster("casting rain of fire!");
            //    //m_ai->SetIgnoreUpdateTime(8);
            //else if (SEARING_PAIN && LastSpellDestruction < 8)
            //    m_ai->CastSpell(SEARING_PAIN, *pTarget);
            //else if (SOUL_FIRE && LastSpellDestruction < 9)
            //    m_ai->CastSpell(SOUL_FIRE, *pTarget);
            //    //m_ai->SetIgnoreUpdateTime(6);
            //else if (HELLFIRE && LastSpellDestruction < 12 && !m_bot->HasAura(HELLFIRE) && m_ai->GetAttackerCount() >= 5 && m_ai->GetHealthPercent() >= 50)
            //    m_ai->CastSpell(HELLFIRE);
            //    m_ai->TellMaster("casting hellfire!");
            //    //m_ai->SetIgnoreUpdateTime(15);
    }

    // No spec due to low level OR no spell found yet
    if (CORRUPTION && m_ai->In_Reach(pTarget,CORRUPTION) && !pTarget->HasAura(CORRUPTION) && CastSpell(CORRUPTION, pTarget))
        return RETURN_CONTINUE;
    if (IMMOLATE && m_ai->In_Reach(pTarget,IMMOLATE) && !pTarget->HasAura(IMMOLATE) && CastSpell(IMMOLATE, pTarget))
        return RETURN_CONTINUE;
    if (SHADOW_BOLT && m_ai->In_Reach(pTarget,SHADOW_BOLT))
        return CastSpell(SHADOW_BOLT, pTarget);

    return RETURN_NO_ACTION_OK;
} // end DoNextCombatManeuver

CombatManeuverReturns PlayerbotWarlockAI::DoNextCombatManeuverPVP(Unit* pTarget)
{
    if (FEAR && m_ai->In_Reach(pTarget,FEAR) && m_ai->CastSpell(FEAR, *pTarget))
        return RETURN_CONTINUE;
    if (SHADOW_BOLT && m_ai->In_Reach(pTarget,SHADOW_BOLT) && m_ai->CastSpell(SHADOW_BOLT))
        return RETURN_CONTINUE;

    return DoNextCombatManeuverPVE(pTarget); // TODO: bad idea perhaps, but better than the alternative
}

// Decision tree for putting a curse on the current target
bool PlayerbotWarlockAI::CheckCurse(Unit* pTarget)
{
    Creature * pCreature = (Creature*) pTarget;
    uint32 CurseToCast = 0;

    // Prevent low health humanoid from fleeing or fleeing too fast
    // Curse of Exhaustion first to avoid increasing damage output on tank
    if (pCreature && pCreature->GetCreatureInfo()->CreatureType == CREATURE_TYPE_HUMANOID && pTarget->GetHealthPercent() < 20 && !pCreature->IsWorldBoss())
    {
        if (CURSE_OF_EXHAUSTION && m_ai->In_Reach(pTarget,CURSE_OF_EXHAUSTION) && !pTarget->HasAura(CURSE_OF_EXHAUSTION))
        {
            if (AMPLIFY_CURSE && !m_bot->HasSpellCooldown(AMPLIFY_CURSE))
                CastSpell(AMPLIFY_CURSE, m_bot);

            if (CastSpell(CURSE_OF_EXHAUSTION, pTarget))
            {
                m_CurrentCurse = CURSE_OF_EXHAUSTION;
                return true;
            }
        }
        else if (CURSE_OF_RECKLESSNESS && m_ai->In_Reach(pTarget,CURSE_OF_RECKLESSNESS) && !pTarget->HasAura(CURSE_OF_RECKLESSNESS) && !pTarget->HasAura(CURSE_OF_EXHAUSTION) && CastSpell(CURSE_OF_RECKLESSNESS, pTarget))
        {
            m_CurrentCurse = CURSE_OF_RECKLESSNESS;
            return true;
        }
    }

    // If bot already put a curse and curse is still active on target: no need to go further
    if (m_CurrentCurse > 0 && pTarget->HasAura(m_CurrentCurse))
        return false;

    // No curse or effect worn off: choose again which curse to use

    // Target is a boss
    if (pCreature && pCreature->IsWorldBoss())
    {
        if (m_bot->GetGroup())
        {
            uint8 mages = 0;
            uint8 warlocks = 1;
            Group::MemberSlotList const& groupSlot = m_bot->GetGroup()->GetMemberSlots();
            for (Group::member_citerator itr = groupSlot.begin(); itr != groupSlot.end(); itr++)
            {
                Player *groupMember = sObjectMgr.GetPlayer(itr->guid);
                if (!groupMember || !groupMember->isAlive())
                    continue;
                switch (groupMember->getClass())
                {
                    case CLASS_WARLOCK:
                        warlocks++;
                        continue;
                    case CLASS_MAGE:
                        mages++;
                        continue;
                }
            }
            if (warlocks > 1 && warlocks > mages)
                CurseToCast = CURSE_OF_SHADOW;
            else if (mages > warlocks)
                CurseToCast = CURSE_OF_THE_ELEMENTS;
            else
                CurseToCast = CURSE_OF_AGONY;
        }
    // If target is not elite, no need to put a curse useful
    // in the long run: go for direct damage
    } else if (!m_ai->IsElite(pTarget))
        CurseToCast = CURSE_OF_AGONY;
    // Enemy elite mages have low health but can cast dangerous spells: group safety before bot DPS
    else if (pCreature && pCreature->GetCreatureInfo()->UnitClass == 8)
        CurseToCast = CURSE_OF_TONGUES;
    // Default case: Curse of Agony
    else
        CurseToCast = CURSE_OF_AGONY;

    // Try to curse the target with the selected curse
    if (CurseToCast && m_ai->In_Reach(pTarget,CurseToCast) && !pTarget->HasAura(CurseToCast))
    {
        if (CurseToCast == CURSE_OF_AGONY)
            if (AMPLIFY_CURSE && !m_bot->HasSpellCooldown(AMPLIFY_CURSE))
                CastSpell(AMPLIFY_CURSE, m_bot);

        if (CastSpell(CurseToCast, pTarget))
        {
            m_CurrentCurse = CurseToCast;
            return true;
        }
    }
    // else: go for Curse of Agony
    else if (CURSE_OF_AGONY && m_ai->In_Reach(pTarget,CURSE_OF_AGONY) && !pTarget->HasAura(CURSE_OF_AGONY))
    {
        if (AMPLIFY_CURSE && !m_bot->HasSpellCooldown(AMPLIFY_CURSE))
            CastSpell(AMPLIFY_CURSE, m_bot);

        if (CastSpell(CURSE_OF_AGONY, pTarget))
        {
            m_CurrentCurse = CURSE_OF_AGONY;
            return true;
        }
    }
    // else: go for Curse of Weakness
    else if (CURSE_OF_WEAKNESS && !pTarget->HasAura(CURSE_OF_WEAKNESS) && !pTarget->HasAura(CURSE_OF_AGONY))
    {
        if (AMPLIFY_CURSE && !m_bot->HasSpellCooldown(AMPLIFY_CURSE))
            CastSpell(AMPLIFY_CURSE, m_bot);

        if (CastSpell(CURSE_OF_WEAKNESS, pTarget))
        {
            m_CurrentCurse = CURSE_OF_WEAKNESS;
            return true;
        }
    }
    else
        return false;
}

void PlayerbotWarlockAI::CheckDemon()
{
    uint32 spec = m_bot->GetSpec();
    uint8 shardCount = m_bot->GetItemCount(SOUL_SHARD, false, nullptr);
    Pet *pet = m_bot->GetPet();
    uint32 demonOfChoice;

    // If pet other than imp is active: return
    if (pet && pet->GetEntry() != DEMON_IMP)
        return;

    //Assign demon of choice
    if (spec == WARLOCK_SPEC_AFFLICTION)
        demonOfChoice = DEMON_FELHUNTER;
    else if (spec == WARLOCK_SPEC_DEMONOLOGY)
        demonOfChoice = DEMON_SUCCUBUS;
    else if (spec == WARLOCK_SPEC_DESTRUCTION)
        demonOfChoice = DEMON_IMP;

    // Summon demon
    if (!pet || m_isTempImp)
    {
        uint32 summonSpellId;
        if (demonOfChoice != DEMON_IMP && shardCount > 0)
        {
            switch (demonOfChoice)
            {
                case DEMON_VOIDWALKER:
                    summonSpellId = SUMMON_VOIDWALKER;
                    break;

                case DEMON_FELHUNTER:
                    summonSpellId = SUMMON_FELHUNTER;
                    break;

                case DEMON_SUCCUBUS:
                    summonSpellId = SUMMON_SUCCUBUS;
                    break;

                default:
                    summonSpellId = 0;
            }

            if (summonSpellId && m_ai->CastSpell(summonSpellId))
            {
                //m_ai->TellMaster("Summoning favorite demon...");
                m_isTempImp = false;
                return;
            }
        }

        if (!pet && SUMMON_IMP && m_ai->CastSpell(SUMMON_IMP))
        {
            if (demonOfChoice != DEMON_IMP)
                m_isTempImp = true;
            else
                m_isTempImp = false;

            //m_ai->TellMaster("Summoning Imp...");
            return;
        }
    }

    return;
}

void PlayerbotWarlockAI::DoNonCombatActions()
{
    if (!m_ai)  return;
    if (!m_bot) return;

    //uint32 spec = m_bot->GetSpec();
    Pet *pet = m_bot->GetPet();

    // Initialize pet spells
    if (pet && pet->GetEntry() != m_lastDemon)
    {
        switch (pet->GetEntry())
        {
            case DEMON_IMP:
                BLOOD_PACT       = m_ai->initPetSpell(BLOOD_PACT_ICON);
                FIREBOLT         = m_ai->initPetSpell(FIREBOLT_ICON);
                FIRE_SHIELD      = m_ai->initPetSpell(FIRE_SHIELD_ICON);
                break;

            case DEMON_VOIDWALKER:
                CONSUME_SHADOWS  = m_ai->initPetSpell(CONSUME_SHADOWS_ICON);
                SACRIFICE        = m_ai->initPetSpell(SACRIFICE_ICON);
                SUFFERING        = m_ai->initPetSpell(SUFFERING_ICON);
                TORMENT          = m_ai->initPetSpell(TORMENT_ICON);
                break;

            case DEMON_SUCCUBUS:
                LASH_OF_PAIN     = m_ai->initPetSpell(LASH_OF_PAIN_ICON);
                SEDUCTION        = m_ai->initPetSpell(SEDUCTION_ICON);
                SOOTHING_KISS    = m_ai->initPetSpell(SOOTHING_KISS_ICON);
                break;

            case DEMON_FELHUNTER:
                DEVOUR_MAGIC     = m_ai->initPetSpell(DEVOUR_MAGIC_ICON);
                SPELL_LOCK       = m_ai->initPetSpell(SPELL_LOCK_ICON);
                break;
        }

        m_lastDemon = pet->GetEntry();
    }

    // Destroy extra soul shards
    uint8 shardCount = m_bot->GetItemCount(SOUL_SHARD, false, nullptr);
    uint8 freeSpace = m_ai->GetFreeBagSpace();
    if (shardCount > MAX_SHARD_COUNT || (freeSpace == 0 && shardCount > 1))
        m_bot->DestroyItemCount(SOUL_SHARD, shardCount > MAX_SHARD_COUNT ? shardCount - MAX_SHARD_COUNT : 1, true, false);

    // buff myself DEMON_SKIN, DEMON_ARMOR, FEL_ARMOR - Strongest one available is chosen
    if (DEMON_ARMOR)
    {
        if (m_ai->SelfBuff(DEMON_ARMOR))
            return;
    }
    else if (DEMON_SKIN)
        if (m_ai->SelfBuff(DEMON_SKIN))
            return;

    // healthstone creation
    if (CREATE_HEALTHSTONE && shardCount > 0)
    {
        Item* const healthStone = m_ai->FindConsumable(HEALTHSTONE_DISPLAYID);
        if (!healthStone && m_ai->CastSpell(CREATE_HEALTHSTONE))
            return;
    }

    // soulstone creation and use
    if (CREATE_SOULSTONE)
    {
        Item* soulStone = m_ai->FindConsumable(SOULSTONE_DISPLAYID);
        if (!soulStone)
        {
            if (shardCount > 0 && !m_bot->HasSpellCooldown(CREATE_SOULSTONE) && m_ai->CastSpell(CREATE_SOULSTONE))
                return;
        }
        else
        {
            uint32 soulStoneSpell = soulStone->GetProto()->Spells[0].SpellId;
            Player* master = GetMaster();
            if (!master->HasAura(soulStoneSpell) && !m_bot->HasSpellCooldown(soulStoneSpell))
            {
                // TODO: first choice: healer. Second choice: anyone else with revive spell. Third choice: self or master.
                m_ai->UseItem(soulStone, master);
                return;
            }
        }
    }

    // hp/mana check
    if (pet && DARK_PACT && (100 * pet->GetPower(POWER_MANA) / pet->GetMaxPower(POWER_MANA)) > 40 && m_ai->GetManaPercent() <= 60)
        if (m_ai->CastSpell(DARK_PACT, *m_bot))
            return;

    if (LIFE_TAP && m_ai->GetManaPercent() <= 80 && m_ai->GetHealthPercent() > 50)
        if (m_ai->CastSpell(LIFE_TAP, *m_bot))
            return;

    // Do not waste time/soul shards to create spellstone or firestone
    // if two-handed weapon (staff) or off-hand item are already equiped
    // Spellstone creation and use (Spellstone dominates firestone completely as I understand it)
    Item* const weapon = m_bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);
    Item* const offweapon = m_bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);
    if (weapon && !offweapon && weapon->GetProto()->SubClass != ITEM_SUBCLASS_WEAPON_STAFF && weapon->GetEnchantmentId(TEMP_ENCHANTMENT_SLOT) == 0)
    {
        Item* const stone = m_ai->FindConsumable(SPELLSTONE_DISPLAYID);
        Item* const stone2 = m_ai->FindConsumable(FIRESTONE_DISPLAYID);
        uint8 spellstone_count = m_bot->GetItemCount(SPELLSTONE, false, nullptr);
        if (spellstone_count == 0)
            spellstone_count = m_bot->GetItemCount(GREATER_SPELLSTONE, false, nullptr);
        if (spellstone_count == 0)
            spellstone_count = m_bot->GetItemCount(MAJOR_SPELLSTONE, false, nullptr);
        uint8 firestone_count = m_bot->GetItemCount(LESSER_FIRESTONE, false, nullptr);
        if (firestone_count == 0)
            firestone_count = m_bot->GetItemCount(FIRESTONE, false, nullptr);
        if (firestone_count == 0)
            firestone_count = m_bot->GetItemCount(GREATER_FIRESTONE, false, nullptr);
        if (firestone_count == 0)
            firestone_count = m_bot->GetItemCount(MAJOR_FIRESTONE, false, nullptr);
        if (spellstone_count == 0 && firestone_count == 0)
        {
            if (CREATE_SPELLSTONE && shardCount > 0 && m_ai->CastSpell(CREATE_SPELLSTONE))
                return;
            else if (CREATE_SPELLSTONE == 0 && CREATE_FIRESTONE > 0 && shardCount > 0 && m_ai->CastSpell(CREATE_FIRESTONE))
                return;
        }
        else if (stone)
        {
            m_ai->UseItem(stone, EQUIPMENT_SLOT_OFFHAND);
            return;
        }
        else
        {
            m_ai->UseItem(stone2, EQUIPMENT_SLOT_OFFHAND);
            return;
        }
    }

    if (EatDrinkBandage())
        return;

    //Heal Voidwalker
    if (pet && pet->GetEntry() == DEMON_VOIDWALKER && CONSUME_SHADOWS && pet->GetHealthPercent() < 75 && !pet->HasAura(CONSUME_SHADOWS))
        m_ai->CastPetSpell(CONSUME_SHADOWS);

    CheckDemon();

    // Soul link demon
    if (pet && SOUL_LINK && !m_bot->HasAura(SOUL_LINK_AURA) && m_ai->CastSpell(SOUL_LINK, *m_bot))
        return;

    // Check demon buffs
    if (pet && pet->GetEntry() == DEMON_IMP && BLOOD_PACT && !m_bot->HasAura(BLOOD_PACT) && m_ai->CastPetSpell(BLOOD_PACT))
        return;
} // end DoNonCombatActions

// Return to UpdateAI the spellId usable to neutralize a target with creaturetype
uint32 PlayerbotWarlockAI::Neutralize(uint8 creatureType)
{
    if (!m_bot)         return 0;
    if (!m_ai)          return 0;
    if (!creatureType)  return 0;

    // TODO: add a way to handle spell cast by pet like Seduction
    if (creatureType != CREATURE_TYPE_DEMON && creatureType != CREATURE_TYPE_ELEMENTAL)
    {
        m_ai->TellMaster("I can't banish that target.");
        return 0;
    }

    if (BANISH)
        return BANISH;
    else
        return 0;

    return 0;
}