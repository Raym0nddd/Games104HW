#include "runtime/function/animation/animation_FSM.h"
#include "runtime/core/base/macro.h"

#include <iostream>
namespace Pilot
{
    AnimationFSM::AnimationFSM() {}
    float tryGetFloat(const json11::Json::object& json, const std::string& key, float default_value)
    {
        auto found_iter = json.find(key);
        if (found_iter != json.end() && found_iter->second.is_number())
        {
            return found_iter->second.number_value();
        }
        return default_value;
    }
    bool tryGetBool(const json11::Json::object& json, const std::string& key, float default_value)
    {
        auto found_iter = json.find(key);
        if (found_iter != json.end() && found_iter->second.is_bool())
        {
            return found_iter->second.bool_value();
        }
        return default_value;
    }
    bool AnimationFSM::update(const json11::Json::object& signals)
    {
        States last_state     = m_state;
        bool   is_clip_finish = tryGetBool(signals, "clip_finish", false);
        bool   is_jumping     = tryGetBool(signals, "jumping", false);
        float  speed          = tryGetFloat(signals, "speed", 0);
        bool   is_moving      = speed > 0.01f;

        // Process each state with clear, sequential logic for maintainability
        switch (m_state)
        {
            // === IDLE STATE ===
            case States::_idle:
                if (is_jumping)
                {
                    m_state = States::_jump_start_from_idle;
                }
                else if (is_moving)
                {
                    m_state = States::_walk_start;
                }
                // else: stay idle
                break;

            // === WALK START (Transition state - cannot be interrupted) ===
            case States::_walk_start:
                if (is_jumping)
                {
                    // Late jump: walk_start animation must complete first
                    m_state = States::_jump_start_from_walk_run;
                }
                else if (is_clip_finish)
                {
                    // Animation complete, transition based on current input
                    if (is_moving)
                    {
                        m_state = States::_walk_run;
                    }
                    else
                    {
                        m_state = States::_walk_stop;
                    }
                }
                // else: stay in walk_start until animation finishes
                break;

            // === WALK RUN (Loop state - immediately responsive) ===
            case States::_walk_run:
                if (is_jumping)
                {
                    m_state = States::_jump_start_from_walk_run;
                }
                else if (!is_moving)
                {
                    m_state = States::_walk_stop;
                }
                // else: continue walking
                break;

            // === WALK STOP (Transition state - LOW LATENCY IMPROVEMENT) ===
            case States::_walk_stop:
                if (is_jumping)
                {
                    m_state = States::_jump_start_from_walk_run;
                }
                else if (is_moving)
                {
                    // CRITICAL: Immediately transition to walk_start without waiting for clip_finish
                    // This ensures low-latency response to user re-engaging movement
                    m_state = States::_walk_start;
                }
                else if (is_clip_finish)
                {
                    m_state = States::_idle;
                }
                // else: stay in walk_stop
                break;

            // === JUMP START FROM IDLE (Transition state) ===
            case States::_jump_start_from_idle:
                if (!is_clip_finish)
                {
                    // Stay in jump_start until animation finishes
                    break;
                }
                // Animation finished, check if still jumping
                if (is_jumping)
                {
                    m_state = States::_jump_loop_from_idle;
                }
                else
                {
                    m_state = States::_idle;
                }
                break;

            // === JUMP LOOP FROM IDLE (Loop state) ===
            case States::_jump_loop_from_idle:
                if (!is_jumping)
                {
                    m_state = States::_jump_end_from_idle;
                }
                // else: continue jumping
                break;

            // === JUMP END FROM IDLE (Transition state) ===
            case States::_jump_end_from_idle:
                if (!is_clip_finish)
                {
                    break;
                }
                // Animation finished, resume based on current ground state
                if (is_moving)
                {
                    m_state = States::_walk_start;
                }
                else
                {
                    m_state = States::_idle;
                }
                break;

            // === JUMP START FROM WALK (Transition state) ===
            case States::_jump_start_from_walk_run:
                if (!is_clip_finish)
                {
                    break;
                }
                // Animation finished, check if still jumping
                if (is_jumping)
                {
                    m_state = States::_jump_loop_from_walk_run;
                }
                else if (is_moving)
                {
                    m_state = States::_walk_run;
                }
                else
                {
                    m_state = States::_walk_stop;
                }
                break;

            // === JUMP LOOP FROM WALK (Loop state) ===
            case States::_jump_loop_from_walk_run:
                if (!is_jumping)
                {
                    m_state = States::_jump_end_from_walk_run;
                }
                // else: continue jumping
                break;

            // === JUMP END FROM WALK (Transition state) ===
            case States::_jump_end_from_walk_run:
                if (!is_clip_finish)
                {
                    break;
                }
                // Animation finished, resume based on ground movement
                if (is_moving)
                {
                    m_state = States::_walk_run;
                }
                else
                {
                    m_state = States::_idle;
                }
                break;

            default:
                break;
        }
        
        LOG_INFO("AnimationFSM state transition: {} -> {}", static_cast<int>(last_state), static_cast<int>(m_state));
        return last_state != m_state;
    }

    std::string AnimationFSM::getCurrentClipBaseName() const
    {
        switch (m_state)
        {
            case States::_idle:
                return "idle_walk_run";
            case States::_walk_start:
                return "walk_start";
            case States::_walk_run:
                return "idle_walk_run";
            case States::_walk_stop:
                return "walk_stop";
            case States::_jump_start_from_walk_run:
            case States::_jump_start_from_idle:
                return "jump_start";
            case States::_jump_loop_from_walk_run:
            case States::_jump_loop_from_idle:
                return "jump_loop";
            case States::_jump_end_from_walk_run:
            case States::_jump_end_from_idle:
                return "jump_stop";
            default:
                return "idle_walk_run";
        }
    }
}

