/*
 * Copyright (c) 2013 Ambroz Bizjak
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef AMBROLIB_PRINTER_MAIN_H
#define AMBROLIB_PRINTER_MAIN_H

#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <stdlib.h>
#include <stddef.h>

#include <aprinter/meta/TypeList.h>
#include <aprinter/meta/MapTypeList.h>
#include <aprinter/meta/TemplateFunc.h>
#include <aprinter/meta/TypeListGet.h>
#include <aprinter/meta/IndexElemTuple.h>
#include <aprinter/meta/TupleGet.h>
#include <aprinter/meta/TupleForEach.h>
#include <aprinter/meta/StructIf.h>
#include <aprinter/meta/WrapDouble.h>
#include <aprinter/meta/ChooseInt.h>
#include <aprinter/meta/TypeListLength.h>
#include <aprinter/meta/BitsInInt.h>
#include <aprinter/meta/IndexElemList.h>
#include <aprinter/meta/Position.h>
#include <aprinter/meta/TuplePosition.h>
#include <aprinter/meta/MakeTypeList.h>
#include <aprinter/meta/JoinTypeLists.h>
#include <aprinter/meta/FixedPoint.h>
#include <aprinter/meta/Union.h>
#include <aprinter/meta/UnionGet.h>
#include <aprinter/meta/GetMemberTypeFunc.h>
#include <aprinter/meta/TypeListFold.h>
#include <aprinter/meta/WrapFunction.h>
#include <aprinter/meta/TypesAreEqual.h>
#include <aprinter/meta/WrapValue.h>
#include <aprinter/meta/TypeListIndex.h>
#include <aprinter/meta/ComposeFunctions.h>
#include <aprinter/meta/IsEqualFunc.h>
#include <aprinter/meta/FilterTypeList.h>
#include <aprinter/meta/NotFunc.h>
#include <aprinter/meta/PowerOfTwo.h>
#include <aprinter/meta/Object.h>
#include <aprinter/base/DebugObject.h>
#include <aprinter/base/Assert.h>
#include <aprinter/base/Lock.h>
#include <aprinter/base/Likely.h>
#include <aprinter/base/ProgramMemory.h>
#include <aprinter/system/InterruptLock.h>
#include <aprinter/math/FloatTools.h>
#include <aprinter/devices/Blinker.h>
#include <aprinter/devices/SoftPwm.h>
#include <aprinter/stepper/Steppers.h>
#include <aprinter/stepper/AxisStepper.h>
#include <aprinter/printer/AxisHomer.h>
#include <aprinter/printer/GcodeParser.h>
#include <aprinter/printer/BinaryGcodeParser.h>
#include <aprinter/printer/MotionPlanner.h>
#include <aprinter/printer/TemperatureObserver.h>

#include <aprinter/BeginNamespace.h>

template <
    typename TSerial, typename TLedPin, typename TLedBlinkInterval, typename TDefaultInactiveTime,
    typename TSpeedLimitMultiply, typename TMaxStepsPerCycle,
    int TStepperSegmentBufferSize, int TEventChannelBufferSize, int TLookaheadBufferSize,
    int TLookaheadCommitCount,
    typename TForceTimeout, typename TFpType,
    template <typename, typename, typename> class TEventChannelTimer,
    template <typename, typename, typename> class TWatchdogTemplate, typename TWatchdogParams,
    typename TSdCardParams, typename TProbeParams, typename TCurrentParams,
    typename TAxesList, typename TTransformParams, typename THeatersList, typename TFansList
>
struct PrinterMainParams {
    using Serial = TSerial;
    using LedPin = TLedPin;
    using LedBlinkInterval = TLedBlinkInterval;
    using DefaultInactiveTime = TDefaultInactiveTime;
    using SpeedLimitMultiply = TSpeedLimitMultiply;
    using MaxStepsPerCycle = TMaxStepsPerCycle;
    static int const StepperSegmentBufferSize = TStepperSegmentBufferSize;
    static int const EventChannelBufferSize = TEventChannelBufferSize;
    static int const LookaheadBufferSize = TLookaheadBufferSize;
    static int const LookaheadCommitCount = TLookaheadCommitCount;
    using ForceTimeout = TForceTimeout;
    using FpType = TFpType;
    template <typename X, typename Y, typename Z> using EventChannelTimer = TEventChannelTimer<X, Y, Z>;
    template <typename X, typename Y, typename Z> using WatchdogTemplate = TWatchdogTemplate<X, Y, Z>;
    using WatchdogParams = TWatchdogParams;
    using SdCardParams = TSdCardParams;
    using ProbeParams = TProbeParams;
    using CurrentParams = TCurrentParams;
    using AxesList = TAxesList;
    using TransformParams = TTransformParams;
    using HeatersList = THeatersList;
    using FansList = TFansList;
};

template <
    uint32_t TBaud,
    int TRecvBufferSizeExp, int TSendBufferSizeExp,
    typename TTheGcodeParserParams,
    template <typename, typename, int, int, typename, typename, typename> class TSerialTemplate,
    typename TSerialParams
>
struct PrinterMainSerialParams {
    static uint32_t const Baud = TBaud;
    static int const RecvBufferSizeExp = TRecvBufferSizeExp;
    static int const SendBufferSizeExp = TSendBufferSizeExp;
    using TheGcodeParserParams = TTheGcodeParserParams;
    template <typename S, typename X, int Y, int Z, typename W, typename Q, typename R> using SerialTemplate = TSerialTemplate<S, X, Y, Z, W, Q, R>;
    using SerialParams = TSerialParams;
};

template <
    char TName,
    typename TDirPin, typename TStepPin, typename TEnablePin, bool TInvertDir,
    typename TDefaultStepsPerUnit, typename TDefaultMin, typename TDefaultMax,
    typename TDefaultMaxSpeed, typename TDefaultMaxAccel,
    typename TDefaultDistanceFactor, typename TDefaultCorneringDistance,
    typename THoming, bool TIsCartesian, int TStepBits,
    typename TTheAxisStepperParams, typename TMicroStep
>
struct PrinterMainAxisParams {
    static char const Name = TName;
    using DirPin = TDirPin;
    using StepPin = TStepPin;
    using EnablePin = TEnablePin;
    static bool const InvertDir = TInvertDir;
    using DefaultStepsPerUnit = TDefaultStepsPerUnit;
    using DefaultMin = TDefaultMin;
    using DefaultMax = TDefaultMax;
    using DefaultMaxSpeed = TDefaultMaxSpeed;
    using DefaultMaxAccel = TDefaultMaxAccel;
    using DefaultDistanceFactor = TDefaultDistanceFactor;
    using DefaultCorneringDistance = TDefaultCorneringDistance;
    using Homing = THoming;
    static bool const IsCartesian = TIsCartesian;
    static int const StepBits = TStepBits;
    using TheAxisStepperParams = TTheAxisStepperParams;
    using MicroStep = TMicroStep;
};

struct PrinterMainNoMicroStepParams {
    static bool const Enabled = false;
};

template <
    template<typename, typename, typename> class TMicroStepTemplate,
    typename TMicroStepParams,
    uint8_t TMicroSteps
>
struct PrinterMainMicroStepParams {
    static bool const Enabled = true;
    template <typename X, typename Y, typename Z> using MicroStepTemplate = TMicroStepTemplate<X, Y, Z>;
    using MicroStepParams = TMicroStepParams;
    static uint8_t const MicroSteps = TMicroSteps;
};

struct PrinterMainNoHomingParams {
    static bool const Enabled = false;
};

template <
    typename TEndPin, typename TEndPinInputMode, bool TEndInvert, bool THomeDir,
    typename TDefaultFastMaxDist, typename TDefaultRetractDist, typename TDefaultSlowMaxDist,
    typename TDefaultFastSpeed, typename TDefaultRetractSpeed, typename TDefaultSlowSpeed
>
struct PrinterMainHomingParams {
    static bool const Enabled = true;
    using EndPin = TEndPin;
    using EndPinInputMode = TEndPinInputMode;
    static bool const EndInvert = TEndInvert;
    static bool const HomeDir = THomeDir;
    using DefaultFastMaxDist = TDefaultFastMaxDist;
    using DefaultRetractDist = TDefaultRetractDist;
    using DefaultSlowMaxDist = TDefaultSlowMaxDist;
    using DefaultFastSpeed = TDefaultFastSpeed;
    using DefaultRetractSpeed = TDefaultRetractSpeed;
    using DefaultSlowSpeed = TDefaultSlowSpeed;
};

struct PrinterMainNoTransformParams {
    static const bool Enabled = false;
};

template <
    typename TVirtAxesList, typename TPhysAxesList,
    typename TSegmentsPerSecond,
    template<typename, typename> class TTransformAlg, typename TTransformAlgParams
>
struct PrinterMainTransformParams {
    static bool const Enabled = true;
    using VirtAxesList = TVirtAxesList;
    using PhysAxesList = TPhysAxesList;
    using SegmentsPerSecond = TSegmentsPerSecond;
    template <typename X, typename Y> using TransformAlg = TTransformAlg<X, Y>;
    using TransformAlgParams = TTransformAlgParams;
};

template <
    char TName, typename TMaxSpeed
>
struct PrinterMainVirtualAxisParams {
    static char const Name = TName;
    using MaxSpeed = TMaxSpeed;
};

template <
    char TName, int TSetMCommand, int TWaitMCommand, int TSetConfigMCommand,
    typename TAdcPin, typename TOutputPin, bool TOutputInvert,
    typename TFormula,
    typename TMinSafeTemp, typename TMaxSafeTemp,
    typename TPulseInterval,
    typename TControlInterval,
    template<typename, typename, typename> class TControl,
    typename TControlParams,
    typename TTheTemperatureObserverParams,
    template<typename, typename, typename> class TTimerTemplate
>
struct PrinterMainHeaterParams {
    static char const Name = TName;
    static int const SetMCommand = TSetMCommand;
    static int const WaitMCommand = TWaitMCommand;
    static int const SetConfigMCommand = TSetConfigMCommand;
    using AdcPin = TAdcPin;
    using OutputPin = TOutputPin;
    static bool const OutputInvert = TOutputInvert;
    using Formula = TFormula;
    using MinSafeTemp = TMinSafeTemp;
    using MaxSafeTemp = TMaxSafeTemp;
    using PulseInterval = TPulseInterval;
    using ControlInterval = TControlInterval;
    template <typename X, typename Y, typename Z> using Control = TControl<X, Y, Z>;
    using ControlParams = TControlParams;
    using TheTemperatureObserverParams = TTheTemperatureObserverParams;
    template <typename X, typename Y, typename Z> using TimerTemplate = TTimerTemplate<X, Y, Z>;
};

template <
    int TSetMCommand, int TOffMCommand,
    typename TOutputPin, bool TOutputInvert, typename TPulseInterval, typename TSpeedMultiply,
    template<typename, typename, typename> class TTimerTemplate
>
struct PrinterMainFanParams {
    static int const SetMCommand = TSetMCommand;
    static int const OffMCommand = TOffMCommand;
    using OutputPin = TOutputPin;
    static bool const OutputInvert = TOutputInvert;
    using PulseInterval = TPulseInterval;
    using SpeedMultiply = TSpeedMultiply;
    template <typename X, typename Y, typename Z> using TimerTemplate = TTimerTemplate<X, Y, Z>;
};

struct PrinterMainNoSdCardParams {
    static bool const Enabled = false;
};

template <
    template<typename, typename, typename, int, typename, typename> class TSdCard,
    typename TSdCardParams,
    template<typename, typename, typename, typename> class TGcodeParserTemplate,
    typename TTheGcodeParserParams, int TReadBufferBlocks,
    int TMaxCommandSize
>
struct PrinterMainSdCardParams {
    static bool const Enabled = true;
    template <typename X, typename Y, typename Z, int R, typename W, typename Q> using SdCard = TSdCard<X, Y, Z, R, W, Q>;
    using SdCardParams = TSdCardParams;
    template <typename X, typename Y, typename Z, typename W> using GcodeParserTemplate = TGcodeParserTemplate<X, Y, Z, W>;
    using TheGcodeParserParams = TTheGcodeParserParams;
    static int const ReadBufferBlocks = TReadBufferBlocks;
    static int const MaxCommandSize = TMaxCommandSize;
};

struct PrinterMainNoProbeParams {
    static bool const Enabled = false;
};

template <
    typename TPlatformAxesList,
    char TProbeAxis,
    typename TProbePin,
    typename TProbePinInputMode,
    bool TProbeInvert,
    typename TProbePlatformOffset,
    typename TProbeStartHeight,
    typename TProbeLowHeight,
    typename TProbeRetractDist,
    typename TProbeMoveSpeed,
    typename TProbeFastSpeed,
    typename TProbeRetractSpeed,
    typename TProbeSlowSpeed,
    typename TProbePoints
>
struct PrinterMainProbeParams {
    static bool const Enabled = true;
    using PlatformAxesList = TPlatformAxesList;
    static char const ProbeAxis = TProbeAxis;
    using ProbePin = TProbePin;
    using ProbePinInputMode = TProbePinInputMode;
    static bool const ProbeInvert = TProbeInvert;
    using ProbePlatformOffset = TProbePlatformOffset;
    using ProbeStartHeight = TProbeStartHeight;
    using ProbeLowHeight = TProbeLowHeight;
    using ProbeRetractDist = TProbeRetractDist;
    using ProbeMoveSpeed = TProbeMoveSpeed;
    using ProbeFastSpeed = TProbeFastSpeed;
    using ProbeRetractSpeed = TProbeRetractSpeed;
    using ProbeSlowSpeed = TProbeSlowSpeed;
    using ProbePoints = TProbePoints;
};

struct PrinterMainNoCurrentParams {
    static bool const Enabled = false;
};

template <
    typename TCurrentAxesList,
    template<typename, typename, typename, typename> class TCurrentTemplate,
    typename TCurrentParams
>
struct PrinterMainCurrentParams {
    static bool const Enabled = true;
    using CurrentAxesList = TCurrentAxesList;
    template <typename X, typename Y, typename Z, typename W> using CurrentTemplate = TCurrentTemplate<X, Y, Z, W>;
    using CurrentParams = TCurrentParams;
};

template <
    char TAxisName,
    typename TParams
>
struct PrinterMainCurrentAxis {
    static char const AxisName = TAxisName;
    using Params = TParams;
};

template <typename Position, typename Context, typename Params>
class PrinterMain
: private DebugObject<Context, void>
{
public:
    struct Object;
    
private:
    AMBRO_MAKE_SELF(Context, PrinterMain, Position)
    AMBRO_DECLARE_TUPLE_FOREACH_HELPER(Foreach_init, init)
    AMBRO_DECLARE_TUPLE_FOREACH_HELPER(Foreach_deinit, deinit)
    AMBRO_DECLARE_TUPLE_FOREACH_HELPER(Foreach_start_homing, start_homing)
    AMBRO_DECLARE_TUPLE_FOREACH_HELPER(Foreach_update_homing_mask, update_homing_mask)
    AMBRO_DECLARE_TUPLE_FOREACH_HELPER(Foreach_enable_stepper, enable_stepper)
    AMBRO_DECLARE_TUPLE_FOREACH_HELPER(Foreach_disable_stepper, disable_stepper)
    AMBRO_DECLARE_TUPLE_FOREACH_HELPER(Foreach_init_new_pos, init_new_pos)
    AMBRO_DECLARE_TUPLE_FOREACH_HELPER(Foreach_collect_new_pos, collect_new_pos)
    AMBRO_DECLARE_TUPLE_FOREACH_HELPER(Foreach_do_move, do_move)
    AMBRO_DECLARE_TUPLE_FOREACH_HELPER(Foreach_limit_axis_move_speed, limit_axis_move_speed)
    AMBRO_DECLARE_TUPLE_FOREACH_HELPER(Foreach_fix_aborted_pos, fix_aborted_pos)
    AMBRO_DECLARE_TUPLE_FOREACH_HELPER(Foreach_append_position, append_position)
    AMBRO_DECLARE_TUPLE_FOREACH_HELPER(Foreach_append_endstop, append_endstop)
    AMBRO_DECLARE_TUPLE_FOREACH_HELPER(Foreach_set_relative_positioning, set_relative_positioning)
    AMBRO_DECLARE_TUPLE_FOREACH_HELPER(Foreach_set_position, set_position)
    AMBRO_DECLARE_TUPLE_FOREACH_HELPER(Foreach_finish_set_position, finish_set_position)
    AMBRO_DECLARE_TUPLE_FOREACH_HELPER(Foreach_append_value, append_value)
    AMBRO_DECLARE_TUPLE_FOREACH_HELPER(Foreach_append_adc_value, append_adc_value)
    AMBRO_DECLARE_TUPLE_FOREACH_HELPER(Foreach_check_command, check_command)
    AMBRO_DECLARE_TUPLE_FOREACH_HELPER(Foreach_emergency, emergency)
    AMBRO_DECLARE_TUPLE_FOREACH_HELPER(Foreach_channel_callback, channel_callback)
    AMBRO_DECLARE_TUPLE_FOREACH_HELPER(Foreach_print_config, print_config)
    AMBRO_DECLARE_TUPLE_FOREACH_HELPER(Foreach_continue_locking_helper, continue_locking_helper)
    AMBRO_DECLARE_TUPLE_FOREACH_HELPER(Foreach_continue_planned_helper, continue_planned_helper)
    AMBRO_DECLARE_TUPLE_FOREACH_HELPER(Foreach_continue_unplanned_helper, continue_unplanned_helper)
    AMBRO_DECLARE_TUPLE_FOREACH_HELPER(Foreach_continue_splitclear_helper, continue_splitclear_helper)
    AMBRO_DECLARE_TUPLE_FOREACH_HELPER(Foreach_finish_locked_helper, finish_locked_helper)
    AMBRO_DECLARE_TUPLE_FOREACH_HELPER(Foreach_run_for_state_command, run_for_state_command)
    AMBRO_DECLARE_TUPLE_FOREACH_HELPER(Foreach_finish_init, finish_init)
    AMBRO_DECLARE_TUPLE_FOREACH_HELPER(Foreach_add_axis, add_axis)
    AMBRO_DECLARE_TUPLE_FOREACH_HELPER(Foreach_get_coord, get_coord)
    AMBRO_DECLARE_TUPLE_FOREACH_HELPER(Foreach_report_height, report_height)
    AMBRO_DECLARE_TUPLE_FOREACH_HELPER(Foreach_clamp_req_phys, clamp_req_phys)
    AMBRO_DECLARE_TUPLE_FOREACH_HELPER(Foreach_clamp_move_phys, clamp_move_phys)
    AMBRO_DECLARE_TUPLE_FOREACH_HELPER(Foreach_prepare_split, prepare_split)
    AMBRO_DECLARE_TUPLE_FOREACH_HELPER(Foreach_compute_split, compute_split)
    AMBRO_DECLARE_TUPLE_FOREACH_HELPER(Foreach_get_final_split, get_final_split)
    AMBRO_DECLARE_TUPLE_FOREACH_HELPER(Foreach_limit_virt_axis_speed, limit_virt_axis_speed)
    AMBRO_DECLARE_TUPLE_FOREACH_HELPER(Foreach_check_current_axis, check_current_axis)
    AMBRO_DECLARE_GET_MEMBER_TYPE_FUNC(GetMemberType_ChannelPayload, ChannelPayload)
    AMBRO_DECLARE_GET_MEMBER_TYPE_FUNC(GetMemberType_EventLoopFastEvents, EventLoopFastEvents)
    AMBRO_DECLARE_GET_MEMBER_TYPE_FUNC(GetMemberType_WrappedAxisName, WrappedAxisName)
    AMBRO_DECLARE_GET_MEMBER_TYPE_FUNC(GetMemberType_WrappedPhysAxisIndex, WrappedPhysAxisIndex)
    
    struct WatchdogPosition;
    template <int AxisIndex> struct AxisPosition;
    template <int AxisIndex> struct HomingFeaturePosition;
    template <int AxisIndex> struct HomingStatePosition;
    struct TransformFeaturePosition;
    struct SerialFeaturePosition;
    struct SdCardFeaturePosition;
    struct PlannerPosition;
    template <int HeaterIndex> struct HeaterPosition;
    template <int FanIndex> struct FanPosition;
    struct ProbeFeaturePosition;
    struct CurrentFeaturePosition;
    
    struct BlinkerHandler;
    template <int AxisIndex> struct PlannerGetAxisStepper;
    struct PlannerPullHandler;
    struct PlannerFinishedHandler;
    struct PlannerAbortedHandler;
    struct PlannerUnderrunCallback;
    struct PlannerChannelCallback;
    template <int AxisIndex> struct PlannerPrestepCallback;
    template <int AxisIndex> struct AxisStepperConsumersList;
    
    using Loop = typename Context::EventLoop;
    using Clock = typename Context::Clock;
    using TimeType = typename Clock::TimeType;
    using FpType = typename Params::FpType;
    using AxesList = typename Params::AxesList;
    using TransformParams = typename Params::TransformParams;
    using HeatersList = typename Params::HeatersList;
    using FansList = typename Params::FansList;
    static const int NumAxes = TypeListLength<AxesList>::value;
    using AxisMaskType = typename ChooseInt<NumAxes, false>::Type;
    using AxisCountType = typename ChooseInt<BitsInInt<NumAxes>::value, false>::Type;
    
    template <typename TheAxis>
    using MakeStepperDef = StepperDef<
        typename TheAxis::DirPin,
        typename TheAxis::StepPin,
        typename TheAxis::EnablePin,
        TheAxis::InvertDir
    >;
    
    using TheWatchdog = typename Params::template WatchdogTemplate<WatchdogPosition, Context, typename Params::WatchdogParams>;
    using TheBlinker = Blinker<Context, Object, typename Params::LedPin, BlinkerHandler>;
    using StepperDefsList = MapTypeList<AxesList, TemplateFunc<MakeStepperDef>>;
    using TheSteppers = Steppers<Context, Object, StepperDefsList>;
    
    static_assert(Params::LedBlinkInterval::value() < TheWatchdog::WatchdogTime / 2.0, "");
    
    enum {COMMAND_IDLE, COMMAND_LOCKING, COMMAND_LOCKED};
    enum {PLANNER_NONE, PLANNER_RUNNING, PLANNER_STOPPING, PLANNER_WAITING, PLANNER_PROBE};
    
    struct MoveBuildState;
    
    template <typename ChannelCommonPosition, typename Channel>
    struct ChannelCommon {
        AMBRO_MAKE_SELF(Context, ChannelCommon, ChannelCommonPosition)
        using TheGcodeParser = typename Channel::TheGcodeParser;
        using GcodePartsSizeType = typename TheGcodeParser::PartsSizeType;
        using GcodeParserPartRef = typename TheGcodeParser::PartRef;
        
        // channel interface
        
        static void init (Context c)
        {
            ChannelCommon *o = self(c);
            o->m_state = COMMAND_IDLE;
            o->m_cmd = false;
        }
        
        static void startCommand (Context c)
        {
            ChannelCommon *o = self(c);
            AMBRO_ASSERT(o->m_state == COMMAND_IDLE)
            AMBRO_ASSERT(!o->m_cmd)
            
            o->m_cmd = true;
            if (gc(c)->getNumParts(c) < 0) {
                AMBRO_PGM_P err = AMBRO_PSTR("unknown error");
                switch (gc(c)->getNumParts(c)) {
                    case TheGcodeParser::ERROR_NO_PARTS: err = AMBRO_PSTR("empty command"); break;
                    case TheGcodeParser::ERROR_TOO_MANY_PARTS: err = AMBRO_PSTR("too many parts"); break;
                    case TheGcodeParser::ERROR_INVALID_PART: err = AMBRO_PSTR("invalid part"); break;
                    case TheGcodeParser::ERROR_CHECKSUM: err = AMBRO_PSTR("incorrect checksum"); break;
                    case TheGcodeParser::ERROR_RECV_OVERRUN: err = AMBRO_PSTR("receive buffer overrun"); break;
                }
                reply_append_pstr(c, AMBRO_PSTR("Error:"));
                reply_append_pstr(c, err);
                reply_append_ch(c, '\n');
                return finishCommand(c);
            }
            if (!Channel::start_command_impl(c)) {
                return finishCommand(c);
            }
            work_command<ChannelCommon>(c);
        }
        
        static void maybePauseLockingCommand (Context c)
        {
            ChannelCommon *o = self(c);
            AMBRO_ASSERT(!o->m_cmd || o->m_state == COMMAND_LOCKING)
            AMBRO_ASSERT(o->m_cmd || o->m_state == COMMAND_IDLE)
            
            o->m_state = COMMAND_IDLE;
        }
        
        static bool maybeResumeLockingCommand (Context c)
        {
            ChannelCommon *o = self(c);
            auto *mob = PrinterMain::Object::self(c);
            AMBRO_ASSERT(o->m_state == COMMAND_IDLE)
            
            if (!o->m_cmd) {
                return false;
            }
            o->m_state = COMMAND_LOCKING;
            if (!mob->unlocked_timer.isSet(c)) {
                mob->unlocked_timer.prependNowNotAlready(c);
            }
            return true;
        }
        
        static void maybeCancelLockingCommand (Context c)
        {
            ChannelCommon *o = self(c);
            AMBRO_ASSERT(o->m_state != COMMAND_LOCKED)
            
            o->m_state = COMMAND_IDLE;
            o->m_cmd = false;
        }
        
        static void finishCommand (Context c, bool no_ok = false)
        {
            ChannelCommon *o = self(c);
            auto *mob = PrinterMain::Object::self(c);
            AMBRO_ASSERT(o->m_cmd)
            AMBRO_ASSERT(o->m_state == COMMAND_IDLE || o->m_state == COMMAND_LOCKED)
            
            Channel::finish_command_impl(c, no_ok);
            o->m_cmd = false;
            if (o->m_state == COMMAND_LOCKED) {
                AMBRO_ASSERT(mob->locked)
                o->m_state = COMMAND_IDLE;
                mob->locked = false;
                if (!mob->unlocked_timer.isSet(c)) {
                    mob->unlocked_timer.prependNowNotAlready(c);
                }
            }
        }
        
        // command interface
        
        static bool tryLockedCommand (Context c)
        {
            ChannelCommon *o = self(c);
            PrinterMain *m = PrinterMain::self(c);
            auto *mob = PrinterMain::Object::self(c);
            AMBRO_ASSERT(o->m_state != COMMAND_LOCKING || !mob->locked)
            AMBRO_ASSERT(o->m_state != COMMAND_LOCKED || mob->locked)
            AMBRO_ASSERT(o->m_cmd)
            
            if (o->m_state == COMMAND_LOCKED) {
                return true;
            }
            if (mob->locked) {
                o->m_state = COMMAND_LOCKING;
                return false;
            }
            o->m_state = COMMAND_LOCKED;
            mob->locked = true;
            return true;
        }
        
        static bool tryUnplannedCommand (Context c)
        {
            PrinterMain *m = PrinterMain::self(c);
            auto *mob = PrinterMain::Object::self(c);
            
            if (!tryLockedCommand(c)) {
                return false;
            }
            AMBRO_ASSERT(mob->planner_state == PLANNER_NONE || mob->planner_state == PLANNER_RUNNING)
            if (mob->planner_state == PLANNER_NONE) {
                return true;
            }
            mob->planner_state = PLANNER_STOPPING;
            if (m->m_planning_pull_pending) {
                m->m_planner.waitFinished(c);
                mob->force_timer.unset(c);
            }
            return false;
        }
        
        static bool tryPlannedCommand (Context c)
        {
            PrinterMain *m = PrinterMain::self(c);
            auto *mob = PrinterMain::Object::self(c);
            
            if (!tryLockedCommand(c)) {
                return false;
            }
            AMBRO_ASSERT(mob->planner_state == PLANNER_NONE || mob->planner_state == PLANNER_RUNNING)
            if (mob->planner_state == PLANNER_NONE) {
                m->m_planner.init(c, false);
                mob->planner_state = PLANNER_RUNNING;
                m->m_planning_pull_pending = false;
                now_active(c);
            }
            if (m->m_planning_pull_pending) {
                return true;
            }
            mob->planner_state = PLANNER_WAITING;
            return false;
        }
        
        static bool trySplitClearCommand (Context c)
        {
            PrinterMain *m = PrinterMain::self(c);
            
            if (!tryLockedCommand(c)) {
                return false;
            }
            return m->m_transform_feature.try_splitclear_command(c);
        }
        
        static bool find_command_param (Context c, char code, GcodeParserPartRef *out_part)
        {
            ChannelCommon *o = self(c);
            AMBRO_ASSERT(o->m_cmd)
            AMBRO_ASSERT(code >= 'A')
            AMBRO_ASSERT(code <= 'Z')
            
            auto num_parts = gc(c)->getNumParts(c);
            for (GcodePartsSizeType i = 0; i < num_parts; i++) {
                GcodeParserPartRef part = gc(c)->getPart(c, i);
                if (gc(c)->getPartCode(c, part) == code) {
                    *out_part = part;
                    return true;
                }
            }
            return false;
        }
        
        static uint32_t get_command_param_uint32 (Context c, char code, uint32_t default_value)
        {
            GcodeParserPartRef part;
            if (!find_command_param(c, code, &part)) {
                return default_value;
            }
            return gc(c)->getPartUint32Value(c, part);
        }
        
        static FpType get_command_param_fp (Context c, char code, FpType default_value)
        {
            GcodeParserPartRef part;
            if (!find_command_param(c, code, &part)) {
                return default_value;
            }
            return gc(c)->template getPartFpValue<FpType>(c, part);
        }
        
        static bool find_command_param_fp (Context c, char code, FpType *out)
        {
            GcodeParserPartRef part;
            if (!find_command_param(c, code, &part)) {
                return false;
            }
            *out = gc(c)->template getPartFpValue<FpType>(c, part);
            return true;
        }
        
        static void reply_poke (Context c)
        {
            Channel::reply_poke_impl(c);
        }
        
        static void reply_append_str (Context c, char const *str)
        {
            Channel::reply_append_buffer_impl(c, str, strlen(str));
        }
        
        static void reply_append_pstr (Context c, AMBRO_PGM_P pstr)
        {
            Channel::reply_append_pbuffer_impl(c, pstr, AMBRO_PGM_STRLEN(pstr));
        }
        
        static void reply_append_ch (Context c, char ch)
        {
            Channel::reply_append_ch_impl(c, ch);
        }
        
        static void reply_append_fp (Context c, FpType x)
        {
            char buf[30];
#if defined(AMBROLIB_AVR)
            uint8_t len = AMBRO_PGM_SPRINTF(buf, AMBRO_PSTR("%g"), x);
            Channel::reply_append_buffer_impl(c, buf, len);
#else        
            FloatToStrSoft(x, buf);
            Channel::reply_append_buffer_impl(c, buf, strlen(buf));
#endif
        }
        
        static void reply_append_uint32 (Context c, uint32_t x)
        {
            char buf[11];
#if defined(AMBROLIB_AVR)
            uint8_t len = AMBRO_PGM_SPRINTF(buf, AMBRO_PSTR("%" PRIu32), x);
#else
            uint8_t len = PrintNonnegativeIntDecimal<uint32_t>(x, buf);
#endif
            Channel::reply_append_buffer_impl(c, buf, len);
        }
        
        static void reply_append_uint16 (Context c, uint16_t x)
        {
            char buf[6];
#if defined(AMBROLIB_AVR)
            uint8_t len = AMBRO_PGM_SPRINTF(buf, AMBRO_PSTR("%" PRIu16), x);
#else
            uint8_t len = PrintNonnegativeIntDecimal<uint16_t>(x, buf);
#endif
            Channel::reply_append_buffer_impl(c, buf, len);
        }
        
        static void reply_append_uint8 (Context c, uint8_t x)
        {
            char buf[4];
#if defined(AMBROLIB_AVR)
            uint8_t len = AMBRO_PGM_SPRINTF(buf, AMBRO_PSTR("%" PRIu8), x);
#else
            uint8_t len = PrintNonnegativeIntDecimal<uint8_t>(x, buf);
#endif
            Channel::reply_append_buffer_impl(c, buf, len);
        }
        
        // helper function to do something for the first channel in the given state
        
        template <typename Obj, typename Func, typename... Args>
        static bool run_for_state_command (Context c, uint8_t state, Obj *obj, Func func, Args... args)
        {
            ChannelCommon *o = self(c);
            PrinterMain *m = PrinterMain::self(c);
            
            if (o->m_state == state) {
                func(obj, c, o, args...);
                return false;
            }
            return true;
        }
        
        static TheGcodeParser * gc (Context c)
        {
            Channel *ch = Channel::self(c);
            return &ch->m_gcode_parser;
        }
        
        uint8_t m_state;
        bool m_cmd;
    };
    
    struct SerialFeature {
        AMBRO_MAKE_SELF(Context, SerialFeature, SerialFeaturePosition)
        struct SerialPosition;
        struct GcodeParserPosition;
        struct ChannelCommonPosition;
        struct SerialRecvHandler;
        struct SerialSendHandler;
        
        using TheSerial = typename Params::Serial::template SerialTemplate<SerialPosition, Context, Params::Serial::RecvBufferSizeExp, Params::Serial::SendBufferSizeExp, typename Params::Serial::SerialParams, SerialRecvHandler, SerialSendHandler>;
        using RecvSizeType = typename TheSerial::RecvSizeType;
        using SendSizeType = typename TheSerial::SendSizeType;
        using TheGcodeParser = GcodeParser<GcodeParserPosition, Context, typename Params::Serial::TheGcodeParserParams, typename RecvSizeType::IntType, GcodeParserTypeSerial>;
        using TheChannelCommon = ChannelCommon<ChannelCommonPosition, SerialFeature>;
        
        static void init (Context c)
        {
            SerialFeature *o = self(c);
            o->m_serial.init(c, Params::Serial::Baud);
            o->m_gcode_parser.init(c);
            o->m_channel_common.init(c);
            o->m_recv_next_error = 0;
            o->m_line_number = 1;
        }
        
        static void deinit (Context c)
        {
            SerialFeature *o = self(c);
            o->m_gcode_parser.deinit(c);
            o->m_serial.deinit(c);
        }
        
        static void serial_recv_handler (TheSerial *, Context c)
        {
            SerialFeature *o = self(c);
            
            if (o->m_channel_common.m_cmd) {
                return;
            }
            if (!o->m_gcode_parser.haveCommand(c)) {
                o->m_gcode_parser.startCommand(c, o->m_serial.recvGetChunkPtr(c), o->m_recv_next_error);
                o->m_recv_next_error = 0;
            }
            bool overrun;
            RecvSizeType avail = o->m_serial.recvQuery(c, &overrun);
            if (o->m_gcode_parser.extendCommand(c, avail.value())) {
                return o->m_channel_common.startCommand(c);
            }
            if (overrun) {
                o->m_serial.recvConsume(c, avail);
                o->m_serial.recvClearOverrun(c);
                o->m_gcode_parser.resetCommand(c);
                o->m_recv_next_error = TheGcodeParser::ERROR_RECV_OVERRUN;
            }
        }
        
        static void serial_send_handler (TheSerial *, Context c)
        {
        }
        
        static bool start_command_impl (Context c)
        {
            SerialFeature *o = self(c);
            AMBRO_ASSERT(o->m_channel_common.m_cmd)
            
            bool is_m110 = (o->m_channel_common.gc(c)->getCmdCode(c) == 'M' && o->m_channel_common.gc(c)->getCmdNumber(c) == 110);
            if (is_m110) {
                o->m_line_number = o->m_channel_common.get_command_param_uint32(c, 'L', (o->m_gcode_parser.getCmd(c)->have_line_number ? o->m_gcode_parser.getCmd(c)->line_number : -1));
            }
            if (o->m_gcode_parser.getCmd(c)->have_line_number) {
                if (o->m_gcode_parser.getCmd(c)->line_number != o->m_line_number) {
                    o->m_channel_common.reply_append_pstr(c, AMBRO_PSTR("Error:Line Number is not Last Line Number+1, Last Line:"));
                    o->m_channel_common.reply_append_uint32(c, (uint32_t)(o->m_line_number - 1));
                    o->m_channel_common.reply_append_ch(c, '\n');
                    return false;
                }
            }
            if (o->m_gcode_parser.getCmd(c)->have_line_number || is_m110) {
                o->m_line_number++;
            }
            return true;
        }
        
        static void finish_command_impl (Context c, bool no_ok)
        {
            SerialFeature *o = self(c);
            AMBRO_ASSERT(o->m_channel_common.m_cmd)
            
            if (!no_ok) {
                o->m_channel_common.reply_append_pstr(c, AMBRO_PSTR("ok\n"));
            }
            o->m_serial.sendPoke(c);
            o->m_serial.recvConsume(c, RecvSizeType::import(o->m_gcode_parser.getLength(c)));
            o->m_serial.recvForceEvent(c);
        }
        
        static void reply_poke_impl (Context c)
        {
            SerialFeature *o = self(c);
            o->m_serial.sendPoke(c);
        }
        
        static void reply_append_buffer_impl (Context c, char const *str, uint8_t length)
        {
            SerialFeature *o = self(c);
            SendSizeType avail = o->m_serial.sendQuery(c);
            if (length > avail.value()) {
                length = avail.value();
            }
            while (length > 0) {
                char *chunk_data = o->m_serial.sendGetChunkPtr(c);
                uint8_t chunk_length = o->m_serial.sendGetChunkLen(c, SendSizeType::import(length)).value();
                memcpy(chunk_data, str, chunk_length);
                o->m_serial.sendProvide(c, SendSizeType::import(chunk_length));
                str += chunk_length;
                length -= chunk_length;
            }
        }
        
        static void reply_append_pbuffer_impl (Context c, AMBRO_PGM_P pstr, uint8_t length)
        {
            SerialFeature *o = self(c);
            SendSizeType avail = o->m_serial.sendQuery(c);
            if (length > avail.value()) {
                length = avail.value();
            }
            while (length > 0) {
                char *chunk_data = o->m_serial.sendGetChunkPtr(c);
                uint8_t chunk_length = o->m_serial.sendGetChunkLen(c, SendSizeType::import(length)).value();
                AMBRO_PGM_MEMCPY(chunk_data, pstr, chunk_length);
                o->m_serial.sendProvide(c, SendSizeType::import(chunk_length));
                pstr += chunk_length;
                length -= chunk_length;
            }
        }
        
        static void reply_append_ch_impl (Context c, char ch)
        {
            SerialFeature *o = self(c);
            if (o->m_serial.sendQuery(c).value() > 0) {
                *o->m_serial.sendGetChunkPtr(c) = ch;
                o->m_serial.sendProvide(c, SendSizeType::import(1));
            }
        }
        
        TheSerial m_serial;
        TheGcodeParser m_gcode_parser;
        TheChannelCommon m_channel_common;
        int8_t m_recv_next_error;
        uint32_t m_line_number;
        
        struct SerialPosition : public MemberPosition<SerialFeaturePosition, TheSerial, &SerialFeature::m_serial> {};
        struct GcodeParserPosition : public MemberPosition<SerialFeaturePosition, TheGcodeParser, &SerialFeature::m_gcode_parser> {};
        struct ChannelCommonPosition : public MemberPosition<SerialFeaturePosition, TheChannelCommon, &SerialFeature::m_channel_common> {};
        struct SerialRecvHandler : public AMBRO_WFUNC_TD(&SerialFeature::serial_recv_handler) {};
        struct SerialSendHandler : public AMBRO_WFUNC_TD(&SerialFeature::serial_send_handler) {};
    };
    
    AMBRO_STRUCT_IF(SdCardFeature, Params::SdCardParams::Enabled) {
        AMBRO_MAKE_SELF(Context, SdCardFeature, SdCardFeaturePosition)
        struct SdCardPosition;
        struct GcodeParserPosition;
        struct ChannelCommonPosition;
        struct SdCardInitHandler;
        struct SdCardCommandHandler;
        
        static const int ReadBufferBlocks = Params::SdCardParams::ReadBufferBlocks;
        static const int MaxCommandSize = Params::SdCardParams::MaxCommandSize;
        static const size_t BlockSize = 512;
        static_assert(ReadBufferBlocks >= 2, "");
        static_assert(MaxCommandSize < BlockSize, "");
        static const size_t BufferBaseSize = ReadBufferBlocks * BlockSize;
        using ParserSizeType = typename ChooseInt<BitsInInt<MaxCommandSize>::value, false>::Type;
        using TheSdCard = typename Params::SdCardParams::template SdCard<SdCardPosition, Context, typename Params::SdCardParams::SdCardParams, 1, SdCardInitHandler, SdCardCommandHandler>;
        using TheGcodeParser = typename Params::SdCardParams::template GcodeParserTemplate<GcodeParserPosition, Context, typename Params::SdCardParams::TheGcodeParserParams, ParserSizeType>;
        using SdCardReadState = typename TheSdCard::ReadState;
        using SdCardChannelCommon = ChannelCommon<ChannelCommonPosition, SdCardFeature>;
        enum {SDCARD_NONE, SDCARD_INITING, SDCARD_INITED, SDCARD_RUNNING, SDCARD_PAUSING};
        
        static void init (Context c)
        {
            SdCardFeature *o = self(c);
            o->m_sdcard.init(c);
            o->m_channel_common.init(c);
            o->m_next_event.init(c, SdCardFeature::next_event_handler);
            o->m_state = SDCARD_NONE;
        }
        
        static void deinit (Context c)
        {
            SdCardFeature *o = self(c);
            if (o->m_state != SDCARD_NONE && o->m_state != SDCARD_INITING) {
                o->m_gcode_parser.deinit(c);
            }
            o->m_next_event.deinit(c);
            o->m_sdcard.deinit(c);
        }
        
        template <typename TheChannelCommon>
        static void finish_init (Context c, TheChannelCommon *cc, uint8_t error_code)
        {
            SdCardFeature *o = self(c);
            
            if (error_code) {
                cc->reply_append_pstr(c, AMBRO_PSTR("SD error "));
                cc->reply_append_uint8(c, error_code);
            } else {
                cc->reply_append_pstr(c, AMBRO_PSTR("SD blocks "));
                cc->reply_append_uint32(c, o->m_sdcard.getCapacityBlocks(c));
            }
            cc->reply_append_ch(c, '\n');
            cc->finishCommand(c);
        }
        
        static void sd_card_init_handler (Context c, uint8_t error_code)
        {
            SdCardFeature *o = self(c);
            AMBRO_ASSERT(o->m_state == SDCARD_INITING)
            
            if (error_code) {
                o->m_state = SDCARD_NONE;
            } else {
                o->m_state = SDCARD_INITED;
                o->m_gcode_parser.init(c);
                o->m_start = 0;
                o->m_length = 0;
                o->m_cmd_offset = 0;
                o->m_sd_block = 0;
            }
            Tuple<ChannelCommonList> dummy;
            TupleForEachForwardInterruptible(&dummy, Foreach_run_for_state_command(), c, COMMAND_LOCKED, o, Foreach_finish_init(), error_code);
        }
        
        static void sd_card_command_handler (Context c)
        {
            SdCardFeature *o = self(c);
            AMBRO_ASSERT(o->m_state == SDCARD_RUNNING || o->m_state == SDCARD_PAUSING)
            AMBRO_ASSERT(o->m_length < BufferBaseSize)
            AMBRO_ASSERT(o->m_sd_block < o->m_sdcard.getCapacityBlocks(c))
            
            bool error;
            if (!o->m_sdcard.checkReadBlock(c, &o->m_read_state, &error)) {
                return;
            }
            o->m_sdcard.unsetEvent(c);
            if (o->m_state == SDCARD_PAUSING) {
                o->m_state = SDCARD_INITED;
                return finish_locked(c);
            }
            if (error) {
                SerialFeature::TheChannelCommon::reply_append_pstr(c, AMBRO_PSTR("//SdRdEr\n"));
                SerialFeature::TheChannelCommon::reply_poke(c);
                return start_read(c);
            }
            o->m_sd_block++;
            if (o->m_length == BufferBaseSize - o->m_start) {
                memcpy(o->m_buffer + BufferBaseSize, o->m_buffer, MaxCommandSize - 1);
            }
            o->m_length += BlockSize;
            if (o->m_length < BufferBaseSize && o->m_sd_block < o->m_sdcard.getCapacityBlocks(c)) {
                start_read(c);
            }
            if (!o->m_channel_common.m_cmd && !o->m_eof) {
                o->m_next_event.prependNowNotAlready(c);
            }
        }
        
        static void next_event_handler (typename Loop::QueuedEvent *, Context c)
        {
            SdCardFeature *o = self(c);
            AMBRO_ASSERT(o->m_state == SDCARD_RUNNING)
            AMBRO_ASSERT(!o->m_channel_common.m_cmd)
            AMBRO_ASSERT(!o->m_eof)
            
            AMBRO_PGM_P eof_str;
            if (!o->m_gcode_parser.haveCommand(c)) {
                o->m_gcode_parser.startCommand(c, (char *)buf_get(c, o->m_start, o->m_cmd_offset), 0);
            }
            ParserSizeType avail = (o->m_length - o->m_cmd_offset > MaxCommandSize) ? MaxCommandSize : (o->m_length - o->m_cmd_offset);
            if (o->m_gcode_parser.extendCommand(c, avail)) {
                if (o->m_gcode_parser.getNumParts(c) == TheGcodeParser::ERROR_EOF) {
                    eof_str = AMBRO_PSTR("//SdEof\n");
                    goto eof;
                }
                return o->m_channel_common.startCommand(c);
            }
            if (avail == MaxCommandSize) {
                eof_str = AMBRO_PSTR("//SdLnEr\n");
                goto eof;
            }
            if (o->m_sd_block == o->m_sdcard.getCapacityBlocks(c)) {
                eof_str = AMBRO_PSTR("//SdEnd\n");
                goto eof;
            }
            return;
        eof:
            SerialFeature::TheChannelCommon::reply_append_pstr(c, eof_str);
            SerialFeature::TheChannelCommon::reply_poke(c);
            o->m_eof = true;
        }
        
        template <typename TheChannelCommon>
        static bool check_command (Context c, TheChannelCommon *cc)
        {
            SdCardFeature *o = self(c);
            PrinterMain *m = PrinterMain::self(c);
            
            if (TypesAreEqual<TheChannelCommon, SdCardChannelCommon>::value) {
                return true;
            }
            if (cc->gc(c)->getCmdNumber(c) == 21) {
                if (!cc->tryUnplannedCommand(c)) {
                    return false;
                }
                if (o->m_state != SDCARD_NONE) {
                    cc->finishCommand(c);
                    return false;
                }
                o->m_sdcard.activate(c);
                o->m_state = SDCARD_INITING;
                return false;
            }
            if (cc->gc(c)->getCmdNumber(c) == 22) {
                if (!cc->tryUnplannedCommand(c)) {
                    return false;
                }
                cc->finishCommand(c);
                AMBRO_ASSERT(o->m_state != SDCARD_INITING)
                AMBRO_ASSERT(o->m_state != SDCARD_PAUSING)
                if (o->m_state == SDCARD_NONE) {
                    return false;
                }
                o->m_gcode_parser.deinit(c);
                o->m_state = SDCARD_NONE;
                o->m_next_event.unset(c);
                o->m_channel_common.maybeCancelLockingCommand(c);
                o->m_sdcard.deactivate(c);
                return false;
            }
            if (cc->gc(c)->getCmdNumber(c) == 24) {
                if (!cc->tryUnplannedCommand(c)) {
                    return false;
                }
                cc->finishCommand(c);
                if (o->m_state != SDCARD_INITED) {
                    return false;
                }
                o->m_state = SDCARD_RUNNING;
                o->m_eof = false;
                if (o->m_length < BufferBaseSize && o->m_sd_block < o->m_sdcard.getCapacityBlocks(c)) {
                    start_read(c);
                }
                if (!o->m_channel_common.maybeResumeLockingCommand(c)) {
                    o->m_next_event.prependNowNotAlready(c);
                }
                return false;
            }
            if (cc->gc(c)->getCmdNumber(c) == 25) {
                if (!cc->tryUnplannedCommand(c)) {
                    return false;
                }
                if (o->m_state != SDCARD_RUNNING) {
                    cc->finishCommand(c);
                    return false;
                }
                o->m_next_event.unset(c);
                o->m_channel_common.maybePauseLockingCommand(c);
                if (o->m_length < BufferBaseSize && o->m_sd_block < o->m_sdcard.getCapacityBlocks(c)) {
                    o->m_state = SDCARD_PAUSING;
                } else {
                    o->m_state = SDCARD_INITED;
                    cc->finishCommand(c);
                }
                return false;
            }
            return true;
        }
        
        static bool start_command_impl (Context c)
        {
            return true;
        }
        
        static void finish_command_impl (Context c, bool no_ok)
        {
            SdCardFeature *o = self(c);
            AMBRO_ASSERT(o->m_channel_common.m_cmd)
            AMBRO_ASSERT(o->m_state == SDCARD_RUNNING)
            AMBRO_ASSERT(!o->m_eof)
            AMBRO_ASSERT(o->m_gcode_parser.getLength(c) <= o->m_length - o->m_cmd_offset)
            
            o->m_next_event.prependNowNotAlready(c);
            o->m_cmd_offset += o->m_gcode_parser.getLength(c);
            if (o->m_cmd_offset >= BlockSize) {
                o->m_start += BlockSize;
                if (o->m_start == BufferBaseSize) {
                    o->m_start = 0;
                }
                o->m_length -= BlockSize;
                o->m_cmd_offset -= BlockSize;
                if (o->m_length == BufferBaseSize - BlockSize && o->m_sd_block < o->m_sdcard.getCapacityBlocks(c)) {
                    start_read(c);
                }
            }
        }
        
        static void reply_poke_impl (Context c)
        {
        }
        
        static void reply_append_buffer_impl (Context c, char const *str, uint8_t length)
        {
        }
        
        static void reply_append_pbuffer_impl (Context c, AMBRO_PGM_P pstr, uint8_t length)
        {
        }
        
        static void reply_append_ch_impl (Context c, char ch)
        {
        }
        
        static uint8_t * buf_get (Context c, size_t start, size_t count)
        {
            SdCardFeature *o = self(c);
            
            static_assert(BufferBaseSize <= SIZE_MAX / 2, "");
            size_t x = start + count;
            if (x >= BufferBaseSize) {
                x -= BufferBaseSize;
            }
            return o->m_buffer + x;
        }
        
        static void start_read (Context c)
        {
            SdCardFeature *o = self(c);
            AMBRO_ASSERT(o->m_length < BufferBaseSize)
            AMBRO_ASSERT(o->m_sd_block < o->m_sdcard.getCapacityBlocks(c))
            
            o->m_sdcard.queueReadBlock(c, o->m_sd_block, buf_get(c, o->m_start, o->m_length), &o->m_read_state);
        }
        
        TheSdCard m_sdcard;
        SdCardChannelCommon m_channel_common;
        typename Loop::QueuedEvent m_next_event;
        uint8_t m_state;
        TheGcodeParser m_gcode_parser;
        SdCardReadState m_read_state;
        size_t m_start;
        size_t m_length;
        size_t m_cmd_offset;
        bool m_eof;
        uint32_t m_sd_block;
        uint8_t m_buffer[BufferBaseSize + (MaxCommandSize - 1)];
        
        struct SdCardPosition : public MemberPosition<SdCardFeaturePosition, TheSdCard, &SdCardFeature::m_sdcard> {};
        struct GcodeParserPosition : public MemberPosition<SdCardFeaturePosition, TheGcodeParser, &SdCardFeature::m_gcode_parser> {};
        struct ChannelCommonPosition : public MemberPosition<SdCardFeaturePosition, SdCardChannelCommon, &SdCardFeature::m_channel_common> {};
        struct SdCardInitHandler : public AMBRO_WFUNC_TD(&SdCardFeature::sd_card_init_handler) {};
        struct SdCardCommandHandler : public AMBRO_WFUNC_TD(&SdCardFeature::sd_card_command_handler) {};
        
        using EventLoopFastEvents = typename TheSdCard::EventLoopFastEvents;
        using SdChannelCommonList = MakeTypeList<SdCardChannelCommon>;
    } AMBRO_STRUCT_ELSE(SdCardFeature) {
        static void init (Context c) {}
        static void deinit (Context c) {}
        template <typename TheChannelCommon>
        static bool check_command (Context c, TheChannelCommon *cc) { return true; }
        using EventLoopFastEvents = EmptyTypeList;
        using SdChannelCommonList = EmptyTypeList;
    };
    
    using ChannelCommonList = JoinTypeLists<
        MakeTypeList<typename SerialFeature::TheChannelCommon>,
        typename SdCardFeature::SdChannelCommonList
    >;
    using ChannelCommonTuple = Tuple<ChannelCommonList>;
    
    template <int TAxisIndex>
    struct Axis {
        static const int AxisIndex = TAxisIndex;
        AMBRO_MAKE_SELF(Context, Axis, AxisPosition<AxisIndex>)
        
        struct AxisStepperPosition;
        struct MicroStepFeaturePosition;
        
        using AxisSpec = TypeListGet<AxesList, AxisIndex>;
        using Stepper = typename TheSteppers::template Stepper<AxisIndex>;
        using TheAxisStepper = AxisStepper<AxisStepperPosition, Context, typename AxisSpec::TheAxisStepperParams, Stepper, AxisStepperConsumersList<AxisIndex>>;
        using StepFixedType = FixedPoint<AxisSpec::StepBits, false, 0>;
        using AbsStepFixedType = FixedPoint<AxisSpec::StepBits - 1, true, 0>;
        static const char AxisName = AxisSpec::Name;
        using WrappedAxisName = WrapInt<AxisName>;
        
        AMBRO_STRUCT_IF(HomingFeature, AxisSpec::Homing::Enabled) {
            struct HomingState {
                AMBRO_MAKE_SELF(Context, HomingState, HomingStatePosition<AxisIndex>)
                struct HomerPosition;
                struct HomerGetAxisStepper;
                struct HomerFinishedHandler;
                
                using Homer = AxisHomer<
                    HomerPosition, Context, TheAxisStepper, AxisSpec::StepBits,
                    typename AxisSpec::DefaultDistanceFactor, typename AxisSpec::DefaultCorneringDistance,
                    Params::StepperSegmentBufferSize, Params::LookaheadBufferSize, FpType,
                    typename AxisSpec::Homing::EndPin,
                    AxisSpec::Homing::EndInvert, AxisSpec::Homing::HomeDir, HomerGetAxisStepper, HomerFinishedHandler
                >;
                
                static TheAxisStepper * homer_get_axis_stepper (Context c)
                {
                    return &Axis::self(c)->m_axis_stepper;
                }
                
                static void homer_finished_handler (Context c, bool success)
                {
                    HomingState *o = self(c);
                    Axis *axis = Axis::self(c);
                    PrinterMain *m = PrinterMain::self(c);
                    auto *mob = PrinterMain::Object::self(c);
                    AMBRO_ASSERT(axis->m_state == AXIS_STATE_HOMING)
                    AMBRO_ASSERT(mob->locked)
                    AMBRO_ASSERT(m->m_homing_rem_axes > 0)
                    
                    o->m_homer.deinit(c);
                    axis->m_req_pos = (AxisSpec::Homing::HomeDir ? axis->max_req_pos() : axis->min_req_pos());
                    axis->m_end_pos = AbsStepFixedType::template importFpSaturatedRound<FpType>(axis->dist_from_real(axis->m_req_pos));
                    axis->m_state = AXIS_STATE_OTHER;
                    m->m_transform_feature.template mark_phys_moved<AxisIndex>(c);
                    m->m_homing_rem_axes--;
                    if (m->m_homing_rem_axes == 0) {
                        homing_finished(c);
                    }
                }
                
                Homer m_homer;
                
                struct HomerPosition : public MemberPosition<HomingStatePosition<AxisIndex>, Homer, &HomingState::m_homer> {};
                struct HomerGetAxisStepper : public AMBRO_WFUNC_TD(&HomingState::homer_get_axis_stepper) {};
                struct HomerFinishedHandler : public AMBRO_WFUNC_TD(&HomingState::homer_finished_handler) {};
            };
            
            template <typename TheHomingFeature>
            using MakeAxisStepperConsumersList = MakeTypeList<typename TheHomingFeature::HomingState::Homer::TheAxisStepperConsumer>;
            
            using EventLoopFastEvents = typename HomingState::Homer::EventLoopFastEvents;
            
            static void init (Context c)
            {
                c.pins()->template setInput<typename AxisSpec::Homing::EndPin, typename AxisSpec::Homing::EndPinInputMode>(c);
            }
            
            static void deinit (Context c)
            {
                Axis *axis = Axis::self(c);
                HomingState *hs = HomingState::self(c);
                if (axis->m_state == AXIS_STATE_HOMING) {
                    hs->m_homer.deinit(c);
                }
            }
            
            static void start_homing (Context c, AxisMaskType mask)
            {
                Axis *axis = Axis::self(c);
                PrinterMain *m = PrinterMain::self(c);
                HomingState *hs = HomingState::self(c);
                AMBRO_ASSERT(axis->m_state == AXIS_STATE_OTHER)
                
                if (!(mask & ((AxisMaskType)1 << AxisIndex))) {
                    return;
                }
                
                typename HomingState::Homer::HomingParams params;
                params.fast_max_dist = StepFixedType::template importFpSaturatedRound<FpType>(dist_from_real((FpType)AxisSpec::Homing::DefaultFastMaxDist::value()));
                params.retract_dist = StepFixedType::template importFpSaturatedRound<FpType>(dist_from_real((FpType)AxisSpec::Homing::DefaultRetractDist::value()));
                params.slow_max_dist = StepFixedType::template importFpSaturatedRound<FpType>(dist_from_real((FpType)AxisSpec::Homing::DefaultSlowMaxDist::value()));
                params.fast_speed = speed_from_real((FpType)AxisSpec::Homing::DefaultFastSpeed::value());
                params.retract_speed = speed_from_real((FpType)AxisSpec::Homing::DefaultRetractSpeed::value());
                params.slow_speed = speed_from_real((FpType)AxisSpec::Homing::DefaultSlowSpeed::value());
                params.max_accel = accel_from_real((FpType)AxisSpec::DefaultMaxAccel::value());
                
                Stepper::enable(c);
                hs->m_homer.init(c, params);
                axis->m_state = AXIS_STATE_HOMING;
                m->m_homing_rem_axes++;
            }
            
            template <typename TheChannelCommon>
            static void append_endstop (Context c, TheChannelCommon *cc)
            {
                bool triggered = c.pins()->template get<typename AxisSpec::Homing::EndPin>(c) != AxisSpec::Homing::EndInvert;
                cc->reply_append_ch(c, ' ');
                cc->reply_append_ch(c, AxisName);
                cc->reply_append_ch(c, ':');
                cc->reply_append_ch(c, (triggered ? '1' : '0'));
            }
            
            static FpType init_position ()
            {
                return AxisSpec::Homing::HomeDir ? max_req_pos() : min_req_pos();
            };
        } AMBRO_STRUCT_ELSE(HomingFeature) {
            struct HomingState {};
            template <typename TheHomingFeature>
            using MakeAxisStepperConsumersList = MakeTypeList<>;
            using EventLoopFastEvents = EmptyTypeList;
            static void init (Context c) {}
            static void deinit (Context c) {}
            static void start_homing (Context c, AxisMaskType mask) {}
            template <typename TheChannelCommon>
            static void append_endstop (Context c, TheChannelCommon *cc) {}
            static FpType init_position () { return 0.0f; }
        };
        
        AMBRO_STRUCT_IF(MicroStepFeature, AxisSpec::MicroStep::Enabled) {
            AMBRO_MAKE_SELF(Context, MicroStepFeature, MicroStepFeaturePosition)
            struct MicroStepPosition;
            using MicroStep = typename AxisSpec::MicroStep::template MicroStepTemplate<MicroStepPosition, Context, typename AxisSpec::MicroStep::MicroStepParams>;
            
            static void init (Context c)
            {
                MicroStepFeature *o = self(c);
                o->m_micro_step.init(c, AxisSpec::MicroStep::MicroSteps);
            }
            
            MicroStep m_micro_step;
            struct MicroStepPosition : public MemberPosition<MicroStepFeaturePosition, MicroStep, &MicroStepFeature::m_micro_step> {};
        } AMBRO_STRUCT_ELSE(MicroStepFeature) {
            static void init (Context c) {}
        };
        
        enum {AXIS_STATE_OTHER, AXIS_STATE_HOMING};
        
        static FpType dist_from_real (FpType x)
        {
            return (x * (FpType)AxisSpec::DefaultStepsPerUnit::value());
        }
        
        static FpType dist_to_real (FpType x)
        {
            return (x * (FpType)(1.0 / AxisSpec::DefaultStepsPerUnit::value()));
        }
        
        static FpType speed_from_real (FpType v)
        {
            return (v * (FpType)(AxisSpec::DefaultStepsPerUnit::value() / Clock::time_freq));
        }
        
        static FpType accel_from_real (FpType a)
        {
            return (a * (FpType)(AxisSpec::DefaultStepsPerUnit::value() / (Clock::time_freq * Clock::time_freq)));
        }
        
        static FpType clamp_req_pos (FpType req)
        {
            return FloatMax(min_req_pos(), FloatMin(max_req_pos(), req));
        }
        
        static FpType min_req_pos ()
        {
            return FloatMax((FpType)AxisSpec::DefaultMin::value(), dist_to_real((FpType)AbsStepFixedType::minValue().template fpValue<FpType>()));
        }
        
        static FpType max_req_pos ()
        {
            return FloatMin((FpType)AxisSpec::DefaultMax::value(), dist_to_real((FpType)AbsStepFixedType::maxValue().template fpValue<FpType>()));
        }
        
        static void init (Context c)
        {
            Axis *o = self(c);
            o->m_axis_stepper.init(c);
            o->m_state = AXIS_STATE_OTHER;
            o->m_homing_feature.init(c);
            o->m_micro_step_feature.init(c);
            o->m_req_pos = HomingFeature::init_position();
            o->m_end_pos = AbsStepFixedType::template importFpSaturatedRound<FpType>(o->dist_from_real(o->m_req_pos));
            o->m_relative_positioning = false;
        }
        
        static void deinit (Context c)
        {
            Axis *o = self(c);
            o->m_homing_feature.deinit(c);
            o->m_axis_stepper.deinit(c);
        }
        
        static void start_homing (Context c, AxisMaskType mask)
        {
            Axis *o = self(c);
            return o->m_homing_feature.start_homing(c, mask);
        }
        
        template <typename TheChannelCommon>
        static void update_homing_mask (Context c, TheChannelCommon *cc, AxisMaskType *mask, typename TheChannelCommon::GcodeParserPartRef part)
        {
            if (AxisSpec::Homing::Enabled && cc->gc(c)->getPartCode(c, part) == AxisName) {
                *mask |= (AxisMaskType)1 << AxisIndex;
            }
        }
        
        static void enable_stepper (Context c)
        {
            Stepper::enable(c);
        }
        
        static void disable_stepper (Context c)
        {
            Stepper::disable(c);
        }
        
        static void update_new_pos (Context c, MoveBuildState *s, FpType req)
        {
            Axis *o = self(c);
            PrinterMain *m = PrinterMain::self(c);
            o->m_req_pos = clamp_req_pos(req);
            if (AxisSpec::IsCartesian) {
                s->seen_cartesian = true;
            }
            m->m_transform_feature.template mark_phys_moved<AxisIndex>(c);
        }
        
        template <typename Src, typename AddDistance, typename PlannerCmd>
        static void do_move (Context c, Src new_pos, AddDistance, FpType *distance_squared, FpType *total_steps, PlannerCmd *cmd)
        {
            Axis *o = self(c);
            AbsStepFixedType new_end_pos = AbsStepFixedType::template importFpSaturatedRound<FpType>(dist_from_real(new_pos.template get<AxisIndex>()));
            bool dir = (new_end_pos >= o->m_end_pos);
            StepFixedType move = StepFixedType::importBits(dir ? 
                ((typename StepFixedType::IntType)new_end_pos.bitsValue() - (typename StepFixedType::IntType)o->m_end_pos.bitsValue()) :
                ((typename StepFixedType::IntType)o->m_end_pos.bitsValue() - (typename StepFixedType::IntType)new_end_pos.bitsValue())
            );
            if (AMBRO_UNLIKELY(move.bitsValue() != 0)) {
                if (AddDistance::value && AxisSpec::IsCartesian) {
                    FpType delta = dist_to_real(move.template fpValue<FpType>());
                    *distance_squared += delta * delta;
                }
                *total_steps += move.template fpValue<FpType>();
                Stepper::enable(c);
            }
            auto *mycmd = TupleGetElem<AxisIndex>(&cmd->axes);
            mycmd->dir = dir;
            mycmd->x = move;
            mycmd->max_v_rec = 1.0f / speed_from_real((FpType)AxisSpec::DefaultMaxSpeed::value());
            mycmd->max_a_rec = 1.0f / accel_from_real((FpType)AxisSpec::DefaultMaxAccel::value());
            o->m_end_pos = new_end_pos;
        }
        
        template <typename PlannerCmd>
        static void limit_axis_move_speed (Context c, FpType time_freq_by_max_speed, PlannerCmd *cmd)
        {
            auto *mycmd = TupleGetElem<AxisIndex>(&cmd->axes);
            mycmd->max_v_rec = FloatMax(mycmd->max_v_rec, time_freq_by_max_speed * (FpType)(1.0 / AxisSpec::DefaultStepsPerUnit::value()));
        }
        
        static void fix_aborted_pos (Context c)
        {
            Axis *o = self(c);
            PrinterMain *m = PrinterMain::self(c);
            using RemStepsType = typename ChooseInt<AxisSpec::StepBits, true>::Type;
            RemStepsType rem_steps = m->m_planner.template countAbortedRemSteps<AxisIndex, RemStepsType>(c);
            if (rem_steps != 0) {
                o->m_end_pos.m_bits.m_int -= rem_steps;
                o->m_req_pos = dist_to_real(o->m_end_pos.template fpValue<FpType>());
                m->m_transform_feature.template mark_phys_moved<AxisIndex>(c);
            }
        }
        
        static void only_set_position (Context c, FpType value)
        {
            Axis *o = self(c);
            o->m_req_pos = clamp_req_pos(value);
            o->m_end_pos = AbsStepFixedType::template importFpSaturatedRound<FpType>(dist_from_real(o->m_req_pos));
        }
        
        static void set_position (Context c, FpType value, bool *seen_virtual)
        {
            PrinterMain *m = PrinterMain::self(c);
            only_set_position(c, value);
            m->m_transform_feature.template mark_phys_moved<AxisIndex>(c);
        }
        
        template <typename TheChannelCommon>
        static void append_endstop (Context c, TheChannelCommon *cc)
        {
            HomingFeature::append_endstop(c, cc);
        }
        
        static void emergency ()
        {
            Stepper::emergency();
        }
        
        using EventLoopFastEvents = typename HomingFeature::EventLoopFastEvents;
        
        TheAxisStepper m_axis_stepper;
        uint8_t m_state;
        HomingFeature m_homing_feature;
        MicroStepFeature m_micro_step_feature;
        AbsStepFixedType m_end_pos;
        FpType m_req_pos;
        FpType m_old_pos;
        bool m_relative_positioning;
        
        struct AxisStepperPosition : public MemberPosition<AxisPosition<AxisIndex>, TheAxisStepper, &Axis::m_axis_stepper> {};
        struct MicroStepFeaturePosition : public MemberPosition<AxisPosition<AxisIndex>, MicroStepFeature, &Axis::m_micro_step_feature> {};
    };
    
    using AxesTuple = IndexElemTuple<AxesList, Axis>;
    
    template <int AxisName>
    using FindAxis = TypeListIndex<
        typename AxesTuple::ElemTypes,
        ComposeFunctions<
            IsEqualFunc<WrapInt<AxisName>>,
            GetMemberType_WrappedAxisName
        >
    >;
    
    template <typename TheAxis>
    using MakePlannerAxisSpec = MotionPlannerAxisSpec<
        typename TheAxis::TheAxisStepper,
        PlannerGetAxisStepper<TheAxis::AxisIndex>,
        TheAxis::AxisSpec::StepBits,
        typename TheAxis::AxisSpec::DefaultDistanceFactor,
        typename TheAxis::AxisSpec::DefaultCorneringDistance,
        PlannerPrestepCallback<TheAxis::AxisIndex>
    >;
    
    AMBRO_STRUCT_IF(TransformFeature, TransformParams::Enabled) {
        AMBRO_MAKE_SELF(Context, TransformFeature, TransformFeaturePosition)
        template <int VirtAxisIndex> struct VirtAxisPosition;
        template <int SecondaryAxisIndex> struct SecondaryAxisPosition;
        
        using VirtAxesList = typename TransformParams::VirtAxesList;
        using PhysAxesList = typename TransformParams::PhysAxesList;
        using TheTransformAlg = typename TransformParams::template TransformAlg<typename TransformParams::TransformAlgParams, FpType>;
        using TheSplitter = typename TheTransformAlg::Splitter;
        static int const NumVirtAxes = TheTransformAlg::NumAxes;
        static_assert(TypeListLength<VirtAxesList>::value == NumVirtAxes, "");
        static_assert(TypeListLength<PhysAxesList>::value == NumVirtAxes, "");
        
        struct PhysReqPosSrc {
            Context m_c;
            template <int Index>
            FpType get () { return Axis<VirtAxis<Index>::PhysAxisIndex>::self(m_c)->m_req_pos; }
        };
        
        struct PhysReqPosDst {
            Context m_c;
            template <int Index>
            void set (FpType x) { Axis<VirtAxis<Index>::PhysAxisIndex>::self(m_c)->m_req_pos = x; }
        };
        
        struct VirtReqPosSrc {
            Context m_c;
            template <int Index>
            FpType get () { return VirtAxis<Index>::self(m_c)->m_req_pos; }
        };
        
        struct VirtReqPosDst {
            Context m_c;
            template <int Index>
            void set (FpType x) { VirtAxis<Index>::self(m_c)->m_req_pos = x; }
        };
        
        struct ArraySrc {
            FpType const *m_arr;
            template <int Index>
            FpType get () { return m_arr[Index]; }
        };
        
        struct PhysArrayDst {
            FpType *m_arr;
            template <int Index>
            void set (FpType x) { m_arr[VirtAxis<Index>::PhysAxisIndex] = x; }
        };
        
        static void init (Context c)
        {
            TransformFeature *o = self(c);
            TupleForEachForward(&o->m_virt_axes, Foreach_init(), c);
            update_virt_from_phys(c);
            o->m_virt_update_pending = false;
            o->m_splitclear_pending = false;
            o->m_splitting = false;
        }
        
        static void update_virt_from_phys (Context c)
        {
            TransformFeature *o = self(c);
            TheTransformAlg::physToVirt(PhysReqPosSrc{c}, VirtReqPosDst{c});
        }
        
        static void handle_virt_move (Context c, FpType time_freq_by_max_speed)
        {
            TransformFeature *o = self(c);
            PrinterMain *m = PrinterMain::self(c);
            auto *mob = PrinterMain::Object::self(c);
            AMBRO_ASSERT(mob->planner_state == PLANNER_RUNNING || mob->planner_state == PLANNER_PROBE)
            AMBRO_ASSERT(m->m_planning_pull_pending)
            AMBRO_ASSERT(o->m_splitting)
            AMBRO_ASSERT(FloatIsPosOrPosZero(time_freq_by_max_speed))
            
            o->m_virt_update_pending = false;
            TheTransformAlg::virtToPhys(VirtReqPosSrc{c}, PhysReqPosDst{c});
            TupleForEachForward(&o->m_virt_axes, Foreach_clamp_req_phys(), c);
            do_pending_virt_update(c);
            FpType distance_squared = 0.0f;
            TupleForEachForward(&o->m_virt_axes, Foreach_prepare_split(), c, &distance_squared);
            TupleForEachForward(&o->m_secondary_axes, Foreach_prepare_split(), c, &distance_squared);
            FpType distance = FloatSqrt(distance_squared);
            FpType base_max_v_rec = TupleForEachForwardAccRes(&o->m_virt_axes, distance * time_freq_by_max_speed, Foreach_limit_virt_axis_speed(), c);
            FpType min_segments_by_distance = (FpType)(TransformParams::SegmentsPerSecond::value() * Clock::time_unit) * time_freq_by_max_speed;
            o->m_splitter.start(distance, base_max_v_rec, min_segments_by_distance);
            do_split(c);
        }
        
        template <int PhysAxisIndex>
        static void mark_phys_moved (Context c)
        {
            TransformFeature *o = self(c);
            if (IsPhysAxisTransformPhys<WrapInt<PhysAxisIndex>>::value) {
                o->m_virt_update_pending = true;
            }
        }
        
        static void do_pending_virt_update (Context c)
        {
            TransformFeature *o = self(c);
            if (AMBRO_UNLIKELY(o->m_virt_update_pending)) {
                o->m_virt_update_pending = false;
                update_virt_from_phys(c);
            }
        }
        
        static bool is_splitting (Context c)
        {
            TransformFeature *o = self(c);
            return o->m_splitting;
        }
        
        static void split_more (Context c)
        {
            TransformFeature *o = self(c);
            PrinterMain *m = PrinterMain::self(c);
            auto *mob = PrinterMain::Object::self(c);
            AMBRO_ASSERT(o->m_splitting)
            AMBRO_ASSERT(mob->planner_state != PLANNER_NONE)
            AMBRO_ASSERT(m->m_planning_pull_pending)
            
            do_split(c);
            if (!o->m_splitting && o->m_splitclear_pending) {
                AMBRO_ASSERT(mob->locked)
                AMBRO_ASSERT(mob->planner_state == PLANNER_RUNNING)
                o->m_splitclear_pending = false;
                ChannelCommonTuple dummy;
                TupleForEachForwardInterruptible(&dummy, Foreach_run_for_state_command(), c, COMMAND_LOCKED, o, Foreach_continue_splitclear_helper());
            }
        }
        
        static bool try_splitclear_command (Context c)
        {
            TransformFeature *o = self(c);
            PrinterMain *m = PrinterMain::self(c);
            auto *mob = PrinterMain::Object::self(c);
            AMBRO_ASSERT(mob->locked)
            AMBRO_ASSERT(!o->m_splitclear_pending)
            
            if (!o->m_splitting) {
                return true;
            }
            o->m_splitclear_pending = true;
            return false;
        }
        
        static void do_split (Context c)
        {
            TransformFeature *o = self(c);
            PrinterMain *m = PrinterMain::self(c);
            auto *mob = PrinterMain::Object::self(c);
            AMBRO_ASSERT(o->m_splitting)
            AMBRO_ASSERT(mob->planner_state != PLANNER_NONE)
            AMBRO_ASSERT(m->m_planning_pull_pending)
            
            do {
                FpType rel_max_v_rec;
                FpType frac;
                FpType move_pos[NumAxes];
                if (o->m_splitter.pull(&rel_max_v_rec, &frac)) {
                    FpType virt_pos[NumVirtAxes];
                    TupleForEachForward(&o->m_virt_axes, Foreach_compute_split(), c, frac, virt_pos);
                    TheTransformAlg::virtToPhys(ArraySrc{virt_pos}, PhysArrayDst{move_pos});
                    TupleForEachForward(&o->m_virt_axes, Foreach_clamp_move_phys(), c, move_pos);
                    TupleForEachForward(&o->m_secondary_axes, Foreach_compute_split(), c, frac, move_pos);
                } else {
                    o->m_splitting = false;
                    TupleForEachForward(&o->m_virt_axes, Foreach_get_final_split(), c, move_pos);
                    TupleForEachForward(&o->m_secondary_axes, Foreach_get_final_split(), c, move_pos);
                }
                PlannerSplitBuffer *cmd = m->m_planner.getBuffer(c);
                FpType total_steps = 0.0f;
                TupleForEachForward(&m->m_axes, Foreach_do_move(), c, ArraySrc{move_pos}, WrapBool<false>(), (FpType *)0, &total_steps, cmd);
                if (total_steps != 0.0f) {
                    cmd->rel_max_v_rec = FloatMax(rel_max_v_rec, total_steps * (FpType)(1.0 / (Params::MaxStepsPerCycle::value() * F_CPU * Clock::time_unit)));
                    m->m_planner.axesCommandDone(c);
                    goto submitted;
                }
            } while (o->m_splitting);
            
            m->m_planner.emptyDone(c);
        submitted:
            submitted_planner_command(c);
        }
        
        template <typename TheChannelCommon>
        static void continue_splitclear_helper (Context c, TheChannelCommon *cc)
        {
            TransformFeature *o = self(c);
            AMBRO_ASSERT(cc->m_state == COMMAND_LOCKED)
            AMBRO_ASSERT(!o->m_splitting)
            AMBRO_ASSERT(!o->m_splitclear_pending)
            
            work_command<TheChannelCommon>(c);
        }
        
        static void handle_set_position (Context c, bool seen_virtual)
        {
            TransformFeature *o = self(c);
            AMBRO_ASSERT(!o->m_splitting)
            
            if (seen_virtual) {
                o->m_virt_update_pending = false;
                TheTransformAlg::virtToPhys(VirtReqPosSrc{c}, PhysReqPosDst{c});
                TupleForEachForward(&o->m_virt_axes, Foreach_finish_set_position(), c);
            }
            do_pending_virt_update(c);
        }
        
        template <int VirtAxisIndex>
        struct VirtAxis {
            AMBRO_MAKE_SELF(Context, VirtAxis, VirtAxisPosition<VirtAxisIndex>)
            using VirtAxisParams = TypeListGet<VirtAxesList, VirtAxisIndex>;
            static int const AxisName = VirtAxisParams::Name;
            static int const PhysAxisIndex = FindAxis<TypeListGet<PhysAxesList, VirtAxisIndex>::value>::value;
            using ThePhysAxis = Axis<PhysAxisIndex>;
            static_assert(!ThePhysAxis::AxisSpec::IsCartesian, "");
            using WrappedPhysAxisIndex = WrapInt<PhysAxisIndex>;
            
            static void init (Context c)
            {
                VirtAxis *o = self(c);
                o->m_relative_positioning = false;
            }
            
            static void update_new_pos (Context c, MoveBuildState *s, FpType req)
            {
                VirtAxis *o = self(c);
                TransformFeature *t = TransformFeature::self(c);
                o->m_req_pos = req;
                t->m_splitting = true;
            }
            
            static void clamp_req_phys (Context c)
            {
                ThePhysAxis *axis = ThePhysAxis::self(c);
                TransformFeature *t = TransformFeature::self(c);
                if (AMBRO_UNLIKELY(!(axis->m_req_pos <= ThePhysAxis::max_req_pos()))) {
                    axis->m_req_pos = ThePhysAxis::max_req_pos();
                    t->m_virt_update_pending = true;
                } else if (AMBRO_UNLIKELY(!(axis->m_req_pos >= ThePhysAxis::min_req_pos()))) {
                    axis->m_req_pos = ThePhysAxis::min_req_pos();
                    t->m_virt_update_pending = true;
                }
            }
            
            static void clamp_move_phys (Context c, FpType *move_pos)
            {
                move_pos[PhysAxisIndex] = ThePhysAxis::clamp_req_pos(move_pos[PhysAxisIndex]);
            }
            
            static void prepare_split (Context c, FpType *distance_squared)
            {
                VirtAxis *o = self(c);
                o->m_delta = o->m_req_pos - o->m_old_pos;
                *distance_squared += o->m_delta * o->m_delta;
            }
            
            static void compute_split (Context c, FpType frac, FpType *virt_pos)
            {
                VirtAxis *o = self(c);
                TransformFeature *t = TransformFeature::self(c);
                virt_pos[VirtAxisIndex] = o->m_old_pos + (frac * o->m_delta);
            }
            
            static void get_final_split (Context c, FpType *move_pos)
            {
                ThePhysAxis *axis = ThePhysAxis::self(c);
                move_pos[PhysAxisIndex] = axis->m_req_pos;
            }
            
            static void set_position (Context c, FpType value, bool *seen_virtual)
            {
                VirtAxis *o = self(c);
                o->m_req_pos = value;
                *seen_virtual = true;
            }
            
            static void finish_set_position (Context c)
            {
                ThePhysAxis *axis = ThePhysAxis::self(c);
                TransformFeature *t = TransformFeature::self(c);
                FpType req = axis->m_req_pos;
                axis->only_set_position(c, req);
                if (axis->m_req_pos != req) {
                    t->m_virt_update_pending = true;
                }
            }
            
            static FpType limit_virt_axis_speed (FpType accum, Context c)
            {
                VirtAxis *o = self(c);
                return FloatMax(accum, FloatAbs(o->m_delta) * (FpType)(Clock::time_freq / VirtAxisParams::MaxSpeed::value()));
            }
            
            FpType m_req_pos;
            FpType m_old_pos;
            FpType m_delta;
            bool m_relative_positioning;
        };
        
        using VirtAxesTuple = IndexElemTuple<VirtAxesList, VirtAxis>;
        
        template <typename PhysAxisIndex>
        using IsPhysAxisTransformPhys = WrapBool<(TypeListIndex<
            typename VirtAxesTuple::ElemTypes,
            ComposeFunctions<
                IsEqualFunc<PhysAxisIndex>,
                GetMemberType_WrappedPhysAxisIndex
            >
        >::value >= 0)>;
        
        using SecondaryAxisIndices = FilterTypeList<
            SequenceList<NumAxes>,
            ComposeFunctions<
                NotFunc,
                TemplateFunc<IsPhysAxisTransformPhys>
            >
        >;
        static int const NumSecondaryAxes = TypeListLength<SecondaryAxisIndices>::value;
        
        template <int SecondaryAxisIndex>
        struct SecondaryAxis {
            AMBRO_MAKE_SELF(Context, SecondaryAxis, SecondaryAxisPosition<SecondaryAxisIndex>)
            static int const AxisIndex = TypeListGet<SecondaryAxisIndices, SecondaryAxisIndex>::value;
            using TheAxis = Axis<AxisIndex>;
            
            static void prepare_split (Context c, FpType *distance_squared)
            {
                TheAxis *axis = TheAxis::self(c);
                if (TheAxis::AxisSpec::IsCartesian) {
                    FpType delta = axis->m_req_pos - axis->m_old_pos;
                    *distance_squared += delta * delta;
                }
            }
            
            static void compute_split (Context c, FpType frac, FpType *move_pos)
            {
                TheAxis *axis = TheAxis::self(c);
                TransformFeature *t = TransformFeature::self(c);
                move_pos[AxisIndex] = axis->m_old_pos + (frac * (axis->m_req_pos - axis->m_old_pos));
            }
            
            static void get_final_split (Context c, FpType *move_pos)
            {
                TheAxis *axis = TheAxis::self(c);
                move_pos[AxisIndex] = axis->m_req_pos;
            }
        };
        
        using SecondaryAxesTuple = IndexElemTuple<SecondaryAxisIndices, SecondaryAxis>;
        
        VirtAxesTuple m_virt_axes;
        SecondaryAxesTuple m_secondary_axes;
        bool m_virt_update_pending;
        bool m_splitclear_pending;
        bool m_splitting;
        TheSplitter m_splitter;
        
        template <int VirtAxisIndex> struct VirtAxisPosition : public TuplePosition<TransformFeaturePosition, VirtAxesTuple, &TransformFeature::m_virt_axes, VirtAxisIndex> {};
        template <int SecondaryAxisIndex> struct SecondaryAxisPosition : public TuplePosition<TransformFeaturePosition, SecondaryAxesTuple, &TransformFeature::m_secondary_axes, SecondaryAxisIndex> {};
    } AMBRO_STRUCT_ELSE(TransformFeature) {
        static int const NumVirtAxes = 0;
        static void init (Context c) {}
        static void handle_virt_move (Context c, FpType time_freq_by_max_speed) {}
        template <int PhysAxisIndex>
        static void mark_phys_moved (Context c) {}
        static void do_pending_virt_update (Context c) {}
        static bool is_splitting (Context c) { return false; }
        static void split_more (Context c) {}
        static bool try_splitclear_command (Context c) { return true; }
        static void handle_set_position (Context c, bool seen_virtual) {}
    };
    
    static int const NumPhysVirtAxes = NumAxes + TransformFeature::NumVirtAxes;
    
    template <bool IsVirt, int PhysVirtAxisIndex>
    struct GetPhysVirtAxisHelper {
        using Type = Axis<PhysVirtAxisIndex>;
    };
    
    template <int PhysVirtAxisIndex>
    struct GetPhysVirtAxisHelper<true, PhysVirtAxisIndex> {
        using Type = typename TransformFeature::template VirtAxis<(PhysVirtAxisIndex - NumAxes)>;
    };
    
    template <int PhysVirtAxisIndex>
    using GetPhysVirtAxis = typename GetPhysVirtAxisHelper<(PhysVirtAxisIndex >= NumAxes), PhysVirtAxisIndex>::Type;
    
    template <int PhysVirtAxisIndex>
    struct PhysVirtAxisHelper {
        using TheAxis = GetPhysVirtAxis<PhysVirtAxisIndex>;
        using WrappedAxisName = WrapInt<TheAxis::AxisName>;
        
        static void init_new_pos (Context c)
        {
            TheAxis *axis = TheAxis::self(c);
            axis->m_old_pos = axis->m_req_pos;
        }
        
        static void update_new_pos (Context c, MoveBuildState *s, FpType req)
        {
            TheAxis::update_new_pos(c, s, req);
        }
        
        template <typename TheChannelCommon>
        static bool collect_new_pos (Context c, TheChannelCommon *cc, MoveBuildState *s, typename TheChannelCommon::GcodeParserPartRef part)
        {
            TheAxis *axis = TheAxis::self(c);
            if (AMBRO_UNLIKELY(cc->gc(c)->getPartCode(c, part) == TheAxis::AxisName)) {
                FpType req = cc->gc(c)->template getPartFpValue<FpType>(c, part);
                if (axis->m_relative_positioning) {
                    req += axis->m_old_pos;
                }
                update_new_pos(c, s, req);
                return false;
            }
            return true;
        }
        
        static void set_relative_positioning (Context c, bool relative)
        {
            TheAxis *axis = TheAxis::self(c);
            axis->m_relative_positioning = relative;
        }
        
        template <typename TheChannelCommon>
        static void append_position (Context c, TheChannelCommon *cc)
        {
            TheAxis *axis = TheAxis::self(c);
            cc->reply_append_ch(c, TheAxis::AxisName);
            cc->reply_append_ch(c, ':');
            cc->reply_append_fp(c, axis->m_req_pos);
        }
        
        template <typename TheChannelCommon>
        static void set_position (Context c, TheChannelCommon *cc, typename TheChannelCommon::GcodeParserPartRef part, bool *seen_virtual)
        {
            if (cc->gc(c)->getPartCode(c, part) == TheAxis::AxisName) {
                FpType value = cc->gc(c)->template getPartFpValue<FpType>(c, part);
                TheAxis::set_position(c, value, seen_virtual);
            }
        }
    };
    
    using PhysVirtAxisHelperTuple = Tuple<IndexElemListCount<NumPhysVirtAxes, PhysVirtAxisHelper>>;
    
    template <int AxisName>
    using FindPhysVirtAxis = TypeListIndex<
        typename PhysVirtAxisHelperTuple::ElemTypes,
        ComposeFunctions<
            IsEqualFunc<WrapInt<AxisName>>,
            GetMemberType_WrappedAxisName
        >
    >;
    
    template <int HeaterIndex>
    struct Heater {
        AMBRO_MAKE_SELF(Context, Heater, HeaterPosition<HeaterIndex>)
        struct SoftPwmTimerHandler;
        struct ObserverGetValueCallback;
        struct ObserverHandler;
        struct SoftPwmPosition;
        struct ObserverPosition;
        
        using HeaterSpec = TypeListGet<HeatersList, HeaterIndex>;
        using TheControl = typename HeaterSpec::template Control<typename HeaterSpec::ControlParams, typename HeaterSpec::ControlInterval, FpType>;
        using ControlConfig = typename TheControl::Config;
        using TheSoftPwm = SoftPwm<SoftPwmPosition, Context, typename HeaterSpec::OutputPin, HeaterSpec::OutputInvert, typename HeaterSpec::PulseInterval, SoftPwmTimerHandler, HeaterSpec::template TimerTemplate>;
        using TheObserver = TemperatureObserver<ObserverPosition, Context, FpType, typename HeaterSpec::TheTemperatureObserverParams, ObserverGetValueCallback, ObserverHandler>;
        using PwmPowerData = typename TheSoftPwm::PowerData;
        using TheFormula = typename HeaterSpec::Formula::template Inner<FpType>;
        using AdcFixedType = typename Context::Adc::FixedType;
        using AdcIntType = typename AdcFixedType::IntType;
        
        static const TimeType ControlIntervalTicks = HeaterSpec::ControlInterval::value() / Clock::time_unit;
        
        // compute the ADC readings corresponding to MinSafeTemp and MaxSafeTemp
        template <typename Temp>
        struct TempToAdcAbs {
            using Result = AMBRO_WRAP_DOUBLE((TheFormula::template TempToAdc<Temp>::Result::value() * PowerOfTwo<double, AdcFixedType::num_bits>::value));
        };
        using InfAdcValueFp = typename TempToAdcAbs<typename HeaterSpec::MaxSafeTemp>::Result;
        using SupAdcValueFp = typename TempToAdcAbs<typename HeaterSpec::MinSafeTemp>::Result;
        static_assert(InfAdcValueFp::value() > 1, "");
        static_assert(SupAdcValueFp::value() < PowerOfTwoMinusOne<AdcIntType, AdcFixedType::num_bits>::value, "");
        static constexpr AdcIntType InfAdcValue = InfAdcValueFp::value();
        static constexpr AdcIntType SupAdcValue = SupAdcValueFp::value();
        
        struct ChannelPayload {
            FpType target;
        };
        
        static void init (Context c)
        {
            Heater *o = self(c);
            o->m_enabled = false;
            o->m_control_config = TheControl::makeDefaultConfig();
            TimeType time = c.clock()->getTime(c) + (TimeType)(0.05 * Clock::time_freq);
            TheSoftPwm::computeZeroPowerData(&o->m_output_pd);
            o->m_control_event.init(c, Heater::control_event_handler);
            o->m_control_event.appendAt(c, time + (TimeType)(0.6 * ControlIntervalTicks));
            o->m_was_not_unset = false;
            o->m_softpwm.init(c, time);
            o->m_observing = false;
        }
        
        static void deinit (Context c)
        {
            Heater *o = self(c);
            if (o->m_observing) {
                o->m_observer.deinit(c);
            }
            o->m_softpwm.deinit(c);
            o->m_control_event.deinit(c);
        }
        
        static FpType adc_to_temp (AdcFixedType adc_value)
        {
            FpType adc_fp = adc_value.template fpValue<FpType>() + (FpType)(0.5 / PowerOfTwo<double, AdcFixedType::num_bits>::value);
            return TheFormula::adc_to_temp(adc_fp);
        }
        
        static FpType get_temp (Context c)
        {
            AdcFixedType adc_value = c.adc()->template getValue<typename HeaterSpec::AdcPin>(c);
            return adc_to_temp(adc_value);
        }
        
        template <typename TheChannelCommon>
        static void append_value (Context c, TheChannelCommon *cc)
        {
            FpType value = get_temp(c);
            cc->reply_append_ch(c, ' ');
            cc->reply_append_ch(c, HeaterSpec::Name);
            cc->reply_append_ch(c, ':');
            cc->reply_append_fp(c, value);
        }
        
        template <typename TheChannelCommon>
        static void append_adc_value (Context c, TheChannelCommon *cc)
        {
            AdcFixedType adc_value = c.adc()->template getValue<typename HeaterSpec::AdcPin>(c);
            cc->reply_append_ch(c, ' ');
            cc->reply_append_ch(c, HeaterSpec::Name);
            cc->reply_append_pstr(c, AMBRO_PSTR("A:"));
            cc->reply_append_fp(c, adc_value.template fpValue<FpType>());
        }
        
        template <typename ThisContext>
        static void set (ThisContext c, FpType target)
        {
            Heater *o = self(c);
            
            AMBRO_LOCK_T(InterruptTempLock(), c, lock_c) {
                o->m_target = target;
                o->m_enabled = true;
            }
        }
        
        template <typename ThisContext>
        static void unset (ThisContext c)
        {
            Heater *o = self(c);
            AMBRO_LOCK_T(InterruptTempLock(), c, lock_c) {
                o->m_enabled = false;
                o->m_was_not_unset = false;
                TheSoftPwm::computeZeroPowerData(&o->m_output_pd);
            }
        }
        
        template <typename TheChannelCommon>
        static bool check_command (Context c, TheChannelCommon *cc)
        {
            Heater *o = self(c);
            PrinterMain *m = PrinterMain::self(c);
            
            if (cc->gc(c)->getCmdNumber(c) == HeaterSpec::WaitMCommand) {
                if (!cc->tryUnplannedCommand(c)) {
                    return false;
                }
                FpType target = cc->get_command_param_fp(c, 'S', 0.0f);
                if (target >= (FpType)HeaterSpec::MinSafeTemp::value() && target <= (FpType)HeaterSpec::MaxSafeTemp::value()) {
                    set(c, target);
                } else {
                    unset(c);
                }
                AMBRO_ASSERT(!o->m_observing)
                o->m_observer.init(c, target);
                o->m_observing = true;
                now_active(c);
                return false;
            }
            if (cc->gc(c)->getCmdNumber(c) == HeaterSpec::SetMCommand) {
                if (!cc->tryPlannedCommand(c)) {
                    return false;
                }
                FpType target = cc->get_command_param_fp(c, 'S', 0.0f);
                cc->finishCommand(c);
                if (!(target >= (FpType)HeaterSpec::MinSafeTemp::value() && target <= (FpType)HeaterSpec::MaxSafeTemp::value())) {
                    target = NAN;
                }
                PlannerSplitBuffer *cmd = m->m_planner.getBuffer(c);
                PlannerChannelPayload *payload = UnionGetElem<0>(&cmd->channel_payload);
                payload->type = HeaterIndex;
                UnionGetElem<HeaterIndex>(&payload->heaters)->target = target;
                m->m_planner.channelCommandDone(c, 1);
                submitted_planner_command(c);
                return false;
            }
            if (cc->gc(c)->getCmdNumber(c) == HeaterSpec::SetConfigMCommand && TheControl::SupportsConfig) {
                if (!cc->tryUnplannedCommand(c)) {
                    return false;
                }
                TheControl::setConfigCommand(c, cc, &o->m_control_config);
                cc->finishCommand(c);
                return false;
            }
            return true;
        }
        
        template <typename TheChannelCommon>
        static void print_config (Context c, TheChannelCommon *cc)
        {
            Heater *o = self(c);
            
            if (TheControl::SupportsConfig) {
                cc->reply_append_pstr(c, AMBRO_PSTR("M" ));
                cc->reply_append_uint32(c, HeaterSpec::SetConfigMCommand);
                TheControl::printConfig(c, cc, &o->m_control_config);
                cc->reply_append_ch(c, '\n');
            }
        }
        
        static void softpwm_timer_handler (typename TheSoftPwm::TimerInstance::HandlerContext c, PwmPowerData *pd)
        {
            Heater *o = self(c);
            
            AdcFixedType adc_value = c.adc()->template getValue<typename HeaterSpec::AdcPin>(c);
            if (AMBRO_LIKELY(adc_value.bitsValue() <= InfAdcValue || adc_value.bitsValue() >= SupAdcValue)) {
                o->m_enabled = false;
                o->m_was_not_unset = false;
                TheSoftPwm::computeZeroPowerData(&o->m_output_pd);
            }
            *pd = o->m_output_pd;
        }
        
        static void observer_handler (Context c, bool state)
        {
            Heater *o = self(c);
            PrinterMain *m = PrinterMain::self(c);
            auto *mob = PrinterMain::Object::self(c);
            AMBRO_ASSERT(o->m_observing)
            AMBRO_ASSERT(mob->locked)
            
            if (!state) {
                return;
            }
            o->m_observer.deinit(c);
            o->m_observing = false;
            now_inactive(c);
            finish_locked(c);
        }
        
        static void emergency ()
        {
            Context::Pins::template emergencySet<typename HeaterSpec::OutputPin>(false);
        }
        
        template <typename ThisContext, typename TheChannelPayloadUnion>
        static void channel_callback (ThisContext c, TheChannelPayloadUnion *payload_union)
        {
            ChannelPayload *payload = UnionGetElem<HeaterIndex>(payload_union);
            if (AMBRO_LIKELY(!isnan(payload->target))) {
                set(c, payload->target);
            } else {
                unset(c);
            }
        }
        
        static void control_event_handler (typename Loop::QueuedEvent *, Context c)
        {
            Heater *o = self(c);
            
            o->m_control_event.appendAfterPrevious(c, ControlIntervalTicks);
            bool enabled;
            FpType target;
            bool was_not_unset;
            AMBRO_LOCK_T(InterruptTempLock(), c, lock_c) {
                enabled = o->m_enabled;
                target = o->m_target;
                was_not_unset = o->m_was_not_unset;
                o->m_was_not_unset = enabled;
            }
            if (AMBRO_LIKELY(enabled)) {
                if (!was_not_unset) {
                    o->m_control.init();
                }
                FpType sensor_value = get_temp(c);
                FpType output = o->m_control.addMeasurement(sensor_value, target, &o->m_control_config);
                PwmPowerData output_pd;
                TheSoftPwm::computePowerData(output, &output_pd);
                AMBRO_LOCK_T(InterruptTempLock(), c, lock_c) {
                    if (o->m_was_not_unset) {
                        o->m_output_pd = output_pd;
                    }
                }
            }
        }
        
        bool m_enabled;
        TheControl m_control;
        ControlConfig m_control_config;
        FpType m_target;
        TheSoftPwm m_softpwm;
        TheObserver m_observer;
        bool m_observing;
        PwmPowerData m_output_pd;
        typename Loop::QueuedEvent m_control_event;
        bool m_was_not_unset;
        
        struct SoftPwmTimerHandler : public AMBRO_WFUNC_TD(&Heater::softpwm_timer_handler) {};
        struct ObserverGetValueCallback : public AMBRO_WFUNC_TD(&Heater::get_temp) {};
        struct ObserverHandler : public AMBRO_WFUNC_TD(&Heater::observer_handler) {};
        struct SoftPwmPosition : public MemberPosition<HeaterPosition<HeaterIndex>, TheSoftPwm, &Heater::m_softpwm> {};
        struct ObserverPosition : public MemberPosition<HeaterPosition<HeaterIndex>, TheObserver, &Heater::m_observer> {};
    };
    
    template <int FanIndex>
    struct Fan {
        AMBRO_MAKE_SELF(Context, Fan, FanPosition<FanIndex>)
        struct SoftPwmTimerHandler;
        struct SoftPwmPosition;
        
        using FanSpec = TypeListGet<FansList, FanIndex>;
        using TheSoftPwm = SoftPwm<SoftPwmPosition, Context, typename FanSpec::OutputPin, FanSpec::OutputInvert, typename FanSpec::PulseInterval, SoftPwmTimerHandler, FanSpec::template TimerTemplate>;
        using PwmPowerData = typename TheSoftPwm::PowerData;
        
        struct ChannelPayload {
            PwmPowerData target_pd;
        };
        
        static void init (Context c)
        {
            Fan *o = self(c);
            TheSoftPwm::computeZeroPowerData(&o->m_target_pd);
            TimeType time = c.clock()->getTime(c) + (TimeType)(0.05 * Clock::time_freq);
            o->m_softpwm.init(c, time);
        }
        
        static void deinit (Context c)
        {
            Fan *o = self(c);
            o->m_softpwm.deinit(c);
        }
        
        template <typename TheChannelCommon>
        static bool check_command (Context c, TheChannelCommon *cc)
        {
            PrinterMain *m = PrinterMain::self(c);
            if (cc->gc(c)->getCmdNumber(c) == FanSpec::SetMCommand || cc->gc(c)->getCmdNumber(c) == FanSpec::OffMCommand) {
                if (!cc->tryPlannedCommand(c)) {
                    return false;
                }
                FpType target = 0.0f;
                if (cc->gc(c)->getCmdNumber(c) == FanSpec::SetMCommand) {
                    target = 1.0f;
                    if (cc->find_command_param_fp(c, 'S', &target)) {
                        target *= (FpType)FanSpec::SpeedMultiply::value();
                    }
                }
                cc->finishCommand(c);
                PlannerSplitBuffer *cmd = m->m_planner.getBuffer(c);
                PlannerChannelPayload *payload = UnionGetElem<0>(&cmd->channel_payload);
                payload->type = TypeListLength<HeatersList>::value + FanIndex;
                TheSoftPwm::computePowerData(target, &UnionGetElem<FanIndex>(&payload->fans)->target_pd);
                m->m_planner.channelCommandDone(c, 1);
                submitted_planner_command(c);
                return false;
            }
            return true;
        }
        
        static void softpwm_timer_handler (typename TheSoftPwm::TimerInstance::HandlerContext c, PwmPowerData *pd)
        {
            Fan *o = self(c);
            AMBRO_LOCK_T(InterruptTempLock(), c, lock_c) {
                *pd = o->m_target_pd;
            }
        }
        
        static void emergency ()
        {
            Context::Pins::template emergencySet<typename FanSpec::OutputPin>(false);
        }
        
        template <typename ThisContext, typename TheChannelPayloadUnion>
        static void channel_callback (ThisContext c, TheChannelPayloadUnion *payload_union)
        {
            Fan *o = self(c);
            ChannelPayload *payload = UnionGetElem<FanIndex>(payload_union);
            AMBRO_LOCK_T(InterruptTempLock(), c, lock_c) {
                o->m_target_pd = payload->target_pd;
            }
        }
        
        PwmPowerData m_target_pd;
        TheSoftPwm m_softpwm;
        
        struct SoftPwmTimerHandler : public AMBRO_WFUNC_TD(&Fan::softpwm_timer_handler) {};
        struct SoftPwmPosition : public MemberPosition<FanPosition<FanIndex>, TheSoftPwm, &Fan::m_softpwm> {};
    };
    
    using HeatersTuple = IndexElemTuple<HeatersList, Heater>;
    using FansTuple = IndexElemTuple<FansList, Fan>;
    
    using HeatersChannelPayloadUnion = Union<MapTypeList<typename HeatersTuple::ElemTypes, GetMemberType_ChannelPayload>>;
    using FansChannelPayloadUnion = Union<MapTypeList<typename FansTuple::ElemTypes, GetMemberType_ChannelPayload>>;
    
    struct PlannerChannelPayload {
        uint8_t type;
        union {
            HeatersChannelPayloadUnion heaters;
            FansChannelPayloadUnion fans;
        };
    };
    
    using MotionPlannerChannels = MakeTypeList<MotionPlannerChannelSpec<PlannerChannelPayload, PlannerChannelCallback, Params::EventChannelBufferSize, Params::template EventChannelTimer>>;
    using MotionPlannerAxes = MapTypeList<IndexElemList<AxesList, Axis>, TemplateFunc<MakePlannerAxisSpec>>;
    using ThePlanner = MotionPlanner<PlannerPosition, Context, MotionPlannerAxes, Params::StepperSegmentBufferSize, Params::LookaheadBufferSize, Params::LookaheadCommitCount, FpType, PlannerPullHandler, PlannerFinishedHandler, PlannerAbortedHandler, PlannerUnderrunCallback, MotionPlannerChannels>;
    using PlannerSplitBuffer = typename ThePlanner::SplitBuffer;
    
    template <int AxisIndex>
    using HomingStateTupleHelper = typename Axis<AxisIndex>::HomingFeature::HomingState;
    using HomingStateTuple = IndexElemTuple<AxesList, HomingStateTupleHelper>;
    
    AMBRO_STRUCT_IF(ProbeFeature, Params::ProbeParams::Enabled) {
        AMBRO_MAKE_SELF(Context, ProbeFeature, ProbeFeaturePosition)
        using ProbeParams = typename Params::ProbeParams;
        static const int NumPoints = TypeListLength<typename ProbeParams::ProbePoints>::value;
        static const int ProbeAxisIndex = FindPhysVirtAxis<Params::ProbeParams::ProbeAxis>::value;
        
        static void init (Context c)
        {
            ProbeFeature *o = self(c);
            o->m_current_point = 0xff;
            c.pins()->template setInput<typename ProbeParams::ProbePin, typename ProbeParams::ProbePinInputMode>(c);
        }
        
        static void deinit (Context c)
        {
            ProbeFeature *o = self(c);
        }
        
        template <typename TheChannelCommon>
        static bool check_command (Context c, TheChannelCommon *cc)
        {
            ProbeFeature *o = self(c);
            if (cc->gc(c)->getCmdNumber(c) == 32) {
                if (!cc->tryUnplannedCommand(c)) {
                    return false;
                }
                AMBRO_ASSERT(o->m_current_point == 0xff)
                init_probe_planner(c, false);
                o->m_current_point = 0;
                o->m_point_state = 0;
                o->m_command_sent = false;
                return false;
            }
            return true;
        }
        
        template <int PlatformAxisIndex>
        struct AxisHelper {
            using PlatformAxis = TypeListGet<typename ProbeParams::PlatformAxesList, PlatformAxisIndex>;
            static const int AxisIndex = FindPhysVirtAxis<PlatformAxis::value>::value;
            using AxisProbeOffset = TypeListGet<typename ProbeParams::ProbePlatformOffset, PlatformAxisIndex>;
            
            static void add_axis (Context c, MoveBuildState *s, uint8_t point_index)
            {
                PointHelperTuple dummy;
                FpType coord = TupleForOne<FpType>(point_index, &dummy, Foreach_get_coord());
                move_add_axis<AxisIndex>(c, s, coord + (FpType)AxisProbeOffset::value());
            }
            
            template <int PointIndex>
            struct PointHelper {
                using Point = TypeListGet<typename ProbeParams::ProbePoints, PointIndex>;
                using PointCoord = TypeListGet<Point, PlatformAxisIndex>;
                
                static FpType get_coord ()
                {
                    return (FpType)PointCoord::value();
                }
            };
            
            using PointHelperTuple = IndexElemTuple<typename ProbeParams::ProbePoints, PointHelper>;
        };
        
        using AxisHelperTuple = IndexElemTuple<typename ProbeParams::PlatformAxesList, AxisHelper>;
        
        static void custom_pull_handler (Context c)
        {
            ProbeFeature *o = self(c);
            AMBRO_ASSERT(o->m_current_point != 0xff)
            AMBRO_ASSERT(o->m_point_state <= 4)
            
            if (o->m_command_sent) {
                custom_planner_wait_finished(c);
                return;
            }
            MoveBuildState s;
            move_begin(c, &s);
            FpType height;
            FpType time_freq_by_speed;
            switch (o->m_point_state) {
                case 0: {
                    AxisHelperTuple dummy;
                    TupleForEachForward(&dummy, Foreach_add_axis(), c, &s, o->m_current_point);
                    height = (FpType)ProbeParams::ProbeStartHeight::value();
                    time_freq_by_speed = (FpType)(Clock::time_freq / ProbeParams::ProbeMoveSpeed::value());
                } break;
                case 1: {
                    height = (FpType)ProbeParams::ProbeLowHeight::value();
                    time_freq_by_speed = (FpType)(Clock::time_freq / ProbeParams::ProbeFastSpeed::value());
                } break;
                case 2: {
                    height = get_height(c) + (FpType)ProbeParams::ProbeRetractDist::value();
                    time_freq_by_speed = (FpType)(Clock::time_freq / ProbeParams::ProbeRetractSpeed::value());
                } break;
                case 3: {
                    height = (FpType)ProbeParams::ProbeLowHeight::value();
                    time_freq_by_speed = (FpType)(Clock::time_freq / ProbeParams::ProbeSlowSpeed::value());
                } break;
                case 4: {
                    height = (FpType)ProbeParams::ProbeStartHeight::value();
                    time_freq_by_speed = (FpType)(Clock::time_freq / ProbeParams::ProbeRetractSpeed::value());
                } break;
            }
            move_add_axis<ProbeAxisIndex>(c, &s, height);
            move_end(c, &s, time_freq_by_speed);
            o->m_command_sent = true;
        }
        
        static void custom_finished_handler (Context c)
        {
            ProbeFeature *o = self(c);
            AMBRO_ASSERT(o->m_current_point != 0xff)
            AMBRO_ASSERT(o->m_command_sent)
            
            custom_planner_deinit(c);
            o->m_command_sent = false;
            if (o->m_point_state < 4) {
                if (o->m_point_state == 3) {
                    FpType height = get_height(c);
                    o->m_samples[o->m_current_point] = height;
                    Tuple<ChannelCommonList> dummy;
                    TupleForEachForwardInterruptible(&dummy, Foreach_run_for_state_command(), c, COMMAND_LOCKED, o, Foreach_report_height(), height);
                }
                o->m_point_state++;
                bool watch_probe = (o->m_point_state == 1 || o->m_point_state == 3);
                init_probe_planner(c, watch_probe);
            } else {
                o->m_current_point++;
                if (o->m_current_point == NumPoints) {
                    o->m_current_point = 0xff;
                    finish_locked(c);
                    return;
                }
                init_probe_planner(c, false);
                o->m_point_state = 0;
            }
        }
        
        static void custom_aborted_handler (Context c)
        {
            ProbeFeature *o = self(c);
            AMBRO_ASSERT(o->m_current_point != 0xff)
            AMBRO_ASSERT(o->m_command_sent)
            AMBRO_ASSERT(o->m_point_state == 1 || o->m_point_state == 3)
            
            custom_finished_handler(c);
        }
        
        template <typename CallbackContext>
        static bool prestep_callback (CallbackContext c)
        {
            return (c.pins()->template get<typename ProbeParams::ProbePin>(c) != Params::ProbeParams::ProbeInvert);
        }
        
        static void init_probe_planner (Context c, bool watch_probe)
        {
            custom_planner_init(c, PLANNER_PROBE, watch_probe);
        }
        
        static FpType get_height (Context c)
        {
            return GetPhysVirtAxis<ProbeAxisIndex>::self(c)->m_req_pos;
        }
        
        template <typename TheChannelCommon>
        static void report_height (Context c, TheChannelCommon *cc, FpType height)
        {
            cc->reply_append_pstr(c, AMBRO_PSTR("//ProbeHeight "));
            cc->reply_append_fp(c, height);
            cc->reply_append_ch(c, '\n');
            cc->reply_poke(c);
        }
        
        uint8_t m_current_point;
        uint8_t m_point_state;
        bool m_command_sent;
        FpType m_samples[NumPoints];
    } AMBRO_STRUCT_ELSE(ProbeFeature) {
        static void init (Context c) {}
        static void deinit (Context c) {}
        template <typename TheChannelCommon>
        static bool check_command (Context c, TheChannelCommon *cc) { return true; }
        static void custom_pull_handler (Context c) {}
        static void custom_finished_handler (Context c) {}
        static void custom_aborted_handler (Context c) {}
        template <typename CallbackContext>
        static bool prestep_callback (CallbackContext c) { return false; }
    };
    
    AMBRO_STRUCT_IF(CurrentFeature, Params::CurrentParams::Enabled) {
        AMBRO_MAKE_SELF(Context, CurrentFeature, CurrentFeaturePosition)
        struct CurrentPosition;
        using CurrentParams = typename Params::CurrentParams;
        using CurrentAxesList = typename CurrentParams::CurrentAxesList;
        template <typename ChannelAxisParams>
        using MakeCurrentChannel = typename ChannelAxisParams::Params;
        using CurrentChannelsList = MapTypeList<CurrentAxesList, TemplateFunc<MakeCurrentChannel>>;
        using Current = typename CurrentParams::template CurrentTemplate<CurrentPosition, Context, typename CurrentParams::CurrentParams, CurrentChannelsList>;
        
        static void init (Context c)
        {
            Current::init(c);
        }
        
        static void deinit (Context c)
        {
            Current::deinit(c);
        }
        
        template <typename TheChannelCommon>
        static bool check_command (Context c, TheChannelCommon *cc)
        {
            if (cc->gc(c)->getCmdNumber(c) == 906) {
                auto num_parts = cc->gc(c)->getNumParts(c);
                for (typename TheChannelCommon::GcodePartsSizeType i = 0; i < num_parts; i++) {
                    typename TheChannelCommon::GcodeParserPartRef part = cc->gc(c)->getPart(c, i);
                    CurrentAxesTuple dummy;
                    TupleForEachForwardInterruptible(&dummy, Foreach_check_current_axis(), c, cc, cc->gc(c)->getPartCode(c, part), cc->gc(c)->template getPartFpValue<FpType>(c, part));
                }
                cc->finishCommand(c);
                return false;
            }
            return true;
        }
        
        using EventLoopFastEvents = typename Current::EventLoopFastEvents;
        
        template <int CurrentAxisIndex>
        struct CurrentAxis {
            using CurrentAxisParams = TypeListGet<CurrentAxesList, CurrentAxisIndex>;
            
            template <typename TheChannelCommon>
            static bool check_current_axis (Context c, TheChannelCommon *cc, char axis_name, FpType current)
            {
                if (axis_name == CurrentAxisParams::AxisName) {
                    Current::template setCurrent<CurrentAxisIndex>(c, current);
                    return false;
                }
                return true;
            }
        };
        
        using CurrentAxesTuple = IndexElemTuple<CurrentAxesList, CurrentAxis>;
        
        Current m_current;
        
        struct CurrentPosition : public MemberPosition<CurrentFeaturePosition, Current, &CurrentFeature::m_current> {};
    } AMBRO_STRUCT_ELSE(CurrentFeature) {
        static void init (Context c) {}
        static void deinit (Context c) {}
        template <typename TheChannelCommon>
        static bool check_command (Context c, TheChannelCommon *cc) { return true; }
        using EventLoopFastEvents = EmptyTypeList;
    };
    
public:
    static void init (Context c)
    {
        PrinterMain *o = self(c);
        auto *ob = Object::self(c);
        
        ob->unlocked_timer.init(c, PrinterMain::unlocked_timer_handler);
        ob->disable_timer.init(c, PrinterMain::disable_timer_handler);
        ob->force_timer.init(c, PrinterMain::force_timer_handler);
        o->m_watchdog.init(c);
        TheBlinker::init(c, (FpType)(Params::LedBlinkInterval::value() * Clock::time_freq));
        TheSteppers::init(c);
        o->m_serial_feature.init(c);
        o->m_sdcard_feature.init(c);
        TupleForEachForward(&o->m_axes, Foreach_init(), c);
        o->m_transform_feature.init(c);
        TupleForEachForward(&o->m_heaters, Foreach_init(), c);
        TupleForEachForward(&o->m_fans, Foreach_init(), c);
        o->m_probe_feature.init(c);
        o->m_current_feature.init(c);
        ob->inactive_time = (FpType)(Params::DefaultInactiveTime::value() * Clock::time_freq);
        ob->time_freq_by_max_speed = 0.0f;
        ob->underrun_count = 0;
        ob->locked = false;
        ob->planner_state = PLANNER_NONE;
        
        o->m_serial_feature.m_channel_common.reply_append_pstr(c, AMBRO_PSTR("start\nAPrinter\n"));
        o->m_serial_feature.m_channel_common.reply_poke(c);
        
        o->debugInit(c);
    }
    
    static void deinit (Context c)
    {
        PrinterMain *o = self(c);
        auto *ob = Object::self(c);
        o->debugDeinit(c);
        
        if (ob->planner_state != PLANNER_NONE) {
            o->m_planner.deinit(c);
        }
        o->m_current_feature.deinit(c);
        o->m_probe_feature.deinit(c);
        TupleForEachReverse(&o->m_fans, Foreach_deinit(), c);
        TupleForEachReverse(&o->m_heaters, Foreach_deinit(), c);
        TupleForEachReverse(&o->m_axes, Foreach_deinit(), c);
        o->m_sdcard_feature.deinit(c);
        o->m_serial_feature.deinit(c);
        TheSteppers::deinit(c);
        TheBlinker::deinit(c);
        o->m_watchdog.deinit(c);
        ob->force_timer.deinit(c);
        ob->disable_timer.deinit(c);
        ob->unlocked_timer.deinit(c);
    }
    
    TheWatchdog * getWatchdog ()
    {
        return &m_watchdog;
    }
    
    typename SerialFeature::TheSerial * getSerial ()
    {
        return &m_serial_feature.m_serial;
    }
    
    template <int AxisIndex>
    typename Axis<AxisIndex>::TheAxisStepper * getAxisStepper ()
    {
        return &TupleGetElem<AxisIndex>(&m_axes)->m_axis_stepper;
    }
    
    template <int HeaterIndex>
    typename Heater<HeaterIndex>::TheSoftPwm::TimerInstance * getHeaterTimer ()
    {
        return TupleGetElem<HeaterIndex>(&m_heaters)->m_softpwm.getTimer();
    }
    
    template <int FanIndex>
    typename Fan<FanIndex>::TheSoftPwm::TimerInstance * getFanTimer ()
    {
        return TupleGetElem<FanIndex>(&m_fans)->m_softpwm.getTimer();
    }
    
    typename ThePlanner::template Channel<0>::TheTimer * getEventChannelTimer ()
    {
        return m_planner.template getChannelTimer<0>();
    }
    
    template <typename TSdCardFeatue = SdCardFeature>
    typename TSdCardFeatue::TheSdCard * getSdCard ()
    {
        return &m_sdcard_feature.m_sdcard;
    }
    
    template <typename TCurrentFeatue = CurrentFeature>
    typename TCurrentFeatue::Current * getCurrent ()
    {
        return &m_current_feature.m_current;
    }
    
    static void emergency ()
    {
        AxesTuple dummy_axes;
        TupleForEachForward(&dummy_axes, Foreach_emergency());
        HeatersTuple dummy_heaters;
        TupleForEachForward(&dummy_heaters, Foreach_emergency());
        FansTuple dummy_fans;
        TupleForEachForward(&dummy_fans, Foreach_emergency());
    }
    
    using EventLoopFastEvents = JoinTypeLists<
        typename CurrentFeature::EventLoopFastEvents,
        JoinTypeLists<
            typename SdCardFeature::EventLoopFastEvents,
            JoinTypeLists<
                typename SerialFeature::TheSerial::EventLoopFastEvents,
                JoinTypeLists<
                    typename ThePlanner::EventLoopFastEvents,
                    TypeListFold<
                        MapTypeList<typename AxesTuple::ElemTypes, GetMemberType_EventLoopFastEvents>,
                        EmptyTypeList,
                        JoinTypeLists
                    >
                >
            >
        >
    >;
    
private:
    static TimeType time_from_real (FpType t)
    {
        return (FixedPoint<30, false, 0>::template importFpSaturatedRound<FpType>(t * (FpType)Clock::time_freq)).bitsValue();
    }
    
    static void blinker_handler (Context c)
    {
        PrinterMain *o = self(c);
        o->debugAccess(c);
        
        o->m_watchdog.reset(c);
    }
    
    template <typename TheChannelCommon>
    static void work_command (Context c)
    {
        PrinterMain *o = self(c);
        auto *ob = Object::self(c);
        TheChannelCommon *cc = TheChannelCommon::self(c);
        AMBRO_ASSERT(cc->m_cmd)
        
        switch (cc->gc(c)->getCmdCode(c)) {
            case 'M': switch (cc->gc(c)->getCmdNumber(c)) {
                default:
                    if (
                        TupleForEachForwardInterruptible(&o->m_heaters, Foreach_check_command(), c, cc) &&
                        TupleForEachForwardInterruptible(&o->m_fans, Foreach_check_command(), c, cc) &&
                        o->m_sdcard_feature.check_command(c, cc) &&
                        o->m_probe_feature.check_command(c, cc) &&
                        o->m_current_feature.check_command(c, cc)
                    ) {
                        goto unknown_command;
                    }
                    return;
                
                case 110: // set line number
                    return cc->finishCommand(c);
                
                case 17: {
                    if (!cc->tryUnplannedCommand(c)) {
                        return;
                    }
                    TupleForEachForward(&o->m_axes, Foreach_enable_stepper(), c);
                    now_inactive(c);
                    return cc->finishCommand(c);
                } break;
                
                case 18: // disable steppers or set timeout
                case 84: {
                    if (!cc->tryUnplannedCommand(c)) {
                        return;
                    }
                    FpType inactive_time;
                    if (cc->find_command_param_fp(c, 'S', &inactive_time)) {
                        ob->inactive_time = time_from_real(inactive_time);
                        if (ob->disable_timer.isSet(c)) {
                            ob->disable_timer.appendAt(c, ob->last_active_time + ob->inactive_time);
                        }
                    } else {
                        TupleForEachForward(&o->m_axes, Foreach_disable_stepper(), c);
                        ob->disable_timer.unset(c);
                    }
                    return cc->finishCommand(c);
                } break;
                
                case 105: {
                    cc->reply_append_pstr(c, AMBRO_PSTR("ok"));
                    TupleForEachForward(&o->m_heaters, Foreach_append_value(), c, cc);
                    cc->reply_append_ch(c, '\n');
                    return cc->finishCommand(c, true);
                } break;
                
                case 114: {
                    PhysVirtAxisHelperTuple dummy;
                    TupleForEachForward(&dummy, Foreach_append_position(), c, cc);
                    cc->reply_append_ch(c, '\n');
                    return cc->finishCommand(c);
                } break;
                
                case 119: {
                    cc->reply_append_pstr(c, AMBRO_PSTR("endstops:"));
                    TupleForEachForward(&o->m_axes, Foreach_append_endstop(), c, cc);
                    cc->reply_append_ch(c, '\n');                    
                    return cc->finishCommand(c, true);
                } break;
                
                case 136: { // print heater config
                    TupleForEachForward(&o->m_heaters, Foreach_print_config(), c, cc);
                    return cc->finishCommand(c);
                } break;
                
#ifdef EVENTLOOP_BENCHMARK
                case 916: { // reset benchmark time
                    if (!cc->tryUnplannedCommand(c)) {
                        return;
                    }
                    c.eventLoop()->resetBenchTime(c);
                    return cc->finishCommand(c);
                } break;
                
                case 917: { // print benchmark time
                    if (!cc->tryUnplannedCommand(c)) {
                        return;
                    }
                    cc->reply_append_uint32(c, c.eventLoop()->getBenchTime(c));
                    cc->reply_append_ch(c, '\n');
                    return cc->finishCommand(c);
                } break;
#endif
                
                case 920: { // get underrun count
                    cc->reply_append_uint32(c, ob->underrun_count);
                    cc->reply_append_ch(c, '\n');
                    cc->finishCommand(c);
                } break;
                
                case 921: { // get heater ADC readings
                    cc->reply_append_pstr(c, AMBRO_PSTR("ok"));
                    TupleForEachForward(&o->m_heaters, Foreach_append_adc_value(), c, cc);
                    cc->reply_append_ch(c, '\n');
                    return cc->finishCommand(c, true);
                } break;
            } break;
            
            case 'G': switch (cc->gc(c)->getCmdNumber(c)) {
                default:
                    goto unknown_command;
                
                case 0:
                case 1: { // buffered move
                    if (!cc->tryPlannedCommand(c)) {
                        return;
                    }
                    MoveBuildState s;
                    move_begin(c, &s);
                    auto num_parts = cc->gc(c)->getNumParts(c);
                    for (typename TheChannelCommon::GcodePartsSizeType i = 0; i < num_parts; i++) {
                        typename TheChannelCommon::GcodeParserPartRef part = cc->gc(c)->getPart(c, i);
                        PhysVirtAxisHelperTuple dummy;
                        if (TupleForEachForwardInterruptible(&dummy, Foreach_collect_new_pos(), c, cc, &s, part)) {
                            if (cc->gc(c)->getPartCode(c, part) == 'F') {
                                ob->time_freq_by_max_speed = (FpType)(Clock::time_freq / Params::SpeedLimitMultiply::value()) / FloatMakePosOrPosZero(cc->gc(c)->template getPartFpValue<FpType>(c, part));
                            }
                        }
                    }
                    cc->finishCommand(c);
                    move_end(c, &s, ob->time_freq_by_max_speed);
                } break;
                
                case 21: // set units to millimeters
                    return cc->finishCommand(c);
                
                case 28: { // home axes
                    if (!cc->tryUnplannedCommand(c)) {
                        return;
                    }
                    AxisMaskType mask = 0;
                    auto num_parts = cc->gc(c)->getNumParts(c);
                    for (typename TheChannelCommon::GcodePartsSizeType i = 0; i < num_parts; i++) {
                        TupleForEachForward(&o->m_axes, Foreach_update_homing_mask(), c, cc, &mask, cc->gc(c)->getPart(c, i));
                    }
                    if (mask == 0) {
                        mask = -1;
                    }
                    o->m_homing_rem_axes = 0;
                    TupleForEachForward(&o->m_axes, Foreach_start_homing(), c, mask);
                    if (o->m_homing_rem_axes == 0) {
                        return cc->finishCommand(c);
                    }
                    now_active(c);
                } break;
                
                case 90: { // absolute positioning
                    PhysVirtAxisHelperTuple dummy;
                    TupleForEachForward(&dummy, Foreach_set_relative_positioning(), c, false);
                    return cc->finishCommand(c);
                } break;
                
                case 91: { // relative positioning
                    PhysVirtAxisHelperTuple dummy;
                    TupleForEachForward(&dummy, Foreach_set_relative_positioning(), c, true);
                    return cc->finishCommand(c);
                } break;
                
                case 92: { // set position
                    if (!cc->trySplitClearCommand(c)) {
                        return;
                    }
                    bool seen_virtual = false;
                    auto num_parts = cc->gc(c)->getNumParts(c);
                    for (typename TheChannelCommon::GcodePartsSizeType i = 0; i < num_parts; i++) {
                        PhysVirtAxisHelperTuple dummy;
                        TupleForEachForward(&dummy, Foreach_set_position(), c, cc, cc->gc(c)->getPart(c, i), &seen_virtual);
                    }
                    o->m_transform_feature.handle_set_position(c, seen_virtual);
                    return cc->finishCommand(c);
                } break;
            } break;
            
            unknown_command:
            default: {
                cc->reply_append_pstr(c, AMBRO_PSTR("Error:Unknown command "));
                cc->reply_append_ch(c, cc->gc(c)->getCmdCode(c));
                cc->reply_append_uint16(c, cc->gc(c)->getCmdNumber(c));
                cc->reply_append_ch(c, '\n');
                return cc->finishCommand(c);
            } break;
        }
    }
    
    template <typename TheChannelCommon>
    static void finish_locked_helper (Context c, TheChannelCommon *cc)
    {
        cc->finishCommand(c);
    }
    
    static void finish_locked (Context c)
    {
        PrinterMain *o = self(c);
        auto *ob = Object::self(c);
        AMBRO_ASSERT(ob->locked)
        
        ChannelCommonTuple dummy;
        TupleForEachForwardInterruptible(&dummy, Foreach_run_for_state_command(), c, COMMAND_LOCKED, o, Foreach_finish_locked_helper());
    }
    
    static void homing_finished (Context c)
    {
        PrinterMain *o = self(c);
        auto *ob = Object::self(c);
        AMBRO_ASSERT(ob->locked)
        AMBRO_ASSERT(o->m_homing_rem_axes == 0)
        
        o->m_transform_feature.do_pending_virt_update(c);
        now_inactive(c);
        finish_locked(c);
    }
    
    static void now_inactive (Context c)
    {
        auto *ob = Object::self(c);
        
        ob->last_active_time = c.clock()->getTime(c);
        ob->disable_timer.appendAt(c, ob->last_active_time + ob->inactive_time);
        TheBlinker::setInterval(c, (FpType)(Params::LedBlinkInterval::value() * Clock::time_freq));
    }
    
    static void now_active (Context c)
    {
        auto *ob = Object::self(c);
        
        ob->disable_timer.unset(c);
        TheBlinker::setInterval(c, (FpType)((Params::LedBlinkInterval::value() / 2) * Clock::time_freq));
    }
    
    static void set_force_timer (Context c)
    {
        PrinterMain *o = self(c);
        auto *ob = Object::self(c);
        AMBRO_ASSERT(ob->planner_state == PLANNER_RUNNING)
        AMBRO_ASSERT(o->m_planning_pull_pending)
        
        TimeType force_time = c.clock()->getTime(c) + (TimeType)(Params::ForceTimeout::value() * Clock::time_freq);
        ob->force_timer.appendAt(c, force_time);
    }
    
    template <typename TheChannelCommon>
    static void continue_locking_helper (Context c, TheChannelCommon *cc)
    {
        auto *ob = Object::self(c);
        AMBRO_ASSERT(!ob->locked)
        AMBRO_ASSERT(cc->m_cmd)
        AMBRO_ASSERT(cc->m_state == COMMAND_LOCKING)
        
        work_command<TheChannelCommon>(c);
    }
    
    static void unlocked_timer_handler (typename Loop::QueuedEvent *, Context c)
    {
        PrinterMain *o = self(c);
        auto *ob = Object::self(c);
        o->debugAccess(c);
        
        if (!ob->locked) {
            ChannelCommonTuple dummy;
            TupleForEachForwardInterruptible(&dummy, Foreach_run_for_state_command(), c, COMMAND_LOCKING, o, Foreach_continue_locking_helper());
        }
    }
    
    static void disable_timer_handler (typename Loop::QueuedEvent *, Context c)
    {
        PrinterMain *o = self(c);
        o->debugAccess(c);
        
        TupleForEachForward(&o->m_axes, Foreach_disable_stepper(), c);
    }
    
    static void force_timer_handler (typename Loop::QueuedEvent *, Context c)
    {
        PrinterMain *o = self(c);
        auto *ob = Object::self(c);
        o->debugAccess(c);
        AMBRO_ASSERT(ob->planner_state == PLANNER_RUNNING)
        AMBRO_ASSERT(o->m_planning_pull_pending)
        
        o->m_planner.waitFinished(c);
    }
    
    template <int AxisIndex>
    static typename Axis<AxisIndex>::TheAxisStepper * planner_get_axis_stepper (Context c)
    {
        return &Axis<AxisIndex>::self(c)->m_axis_stepper;
    }
    
    template <typename TheChannelCommon>
    static void continue_planned_helper (Context c, TheChannelCommon *cc)
    {
        PrinterMain *o = self(c);
        auto *ob = Object::self(c);
        AMBRO_ASSERT(ob->locked)
        AMBRO_ASSERT(ob->planner_state == PLANNER_RUNNING)
        AMBRO_ASSERT(o->m_planning_pull_pending)
        AMBRO_ASSERT(cc->m_state == COMMAND_LOCKED)
        AMBRO_ASSERT(cc->m_cmd)
        
        work_command<TheChannelCommon>(c);
    }
    
    static void planner_pull_handler (Context c)
    {
        PrinterMain *o = self(c);
        auto *ob = Object::self(c);
        o->debugAccess(c);
        AMBRO_ASSERT(ob->planner_state != PLANNER_NONE)
        AMBRO_ASSERT(!o->m_planning_pull_pending)
        
        o->m_planning_pull_pending = true;
        if (o->m_transform_feature.is_splitting(c)) {
            o->m_transform_feature.split_more(c);
            return;
        }
        if (ob->planner_state == PLANNER_STOPPING) {
            o->m_planner.waitFinished(c);
        } else if (ob->planner_state == PLANNER_WAITING) {
            ob->planner_state = PLANNER_RUNNING;
            ChannelCommonTuple dummy;
            TupleForEachForwardInterruptible(&dummy, Foreach_run_for_state_command(), c, COMMAND_LOCKED, o, Foreach_continue_planned_helper());
        } else if (ob->planner_state == PLANNER_RUNNING) {
            set_force_timer(c);
        } else {
            AMBRO_ASSERT(ob->planner_state == PLANNER_PROBE)
            o->m_probe_feature.custom_pull_handler(c);
        }
    }
    
    template <typename TheChannelCommon>
    static void continue_unplanned_helper (Context c, TheChannelCommon *cc)
    {
        auto *ob = Object::self(c);
        AMBRO_ASSERT(ob->locked)
        AMBRO_ASSERT(ob->planner_state == PLANNER_NONE)
        AMBRO_ASSERT(cc->m_state == COMMAND_LOCKED)
        AMBRO_ASSERT(cc->m_cmd)
        
        work_command<TheChannelCommon>(c);
    }
    
    static void planner_finished_handler (Context c)
    {
        PrinterMain *o = self(c);
        auto *ob = Object::self(c);
        o->debugAccess(c);
        AMBRO_ASSERT(ob->planner_state != PLANNER_NONE)
        AMBRO_ASSERT(o->m_planning_pull_pending)
        AMBRO_ASSERT(ob->planner_state != PLANNER_WAITING)
        
        if (ob->planner_state == PLANNER_PROBE) {
            return o->m_probe_feature.custom_finished_handler(c);
        }
        
        uint8_t old_state = ob->planner_state;
        o->m_planner.deinit(c);
        ob->force_timer.unset(c);
        ob->planner_state = PLANNER_NONE;
        now_inactive(c);
        
        if (old_state == PLANNER_STOPPING) {
            ChannelCommonTuple dummy;
            TupleForEachForwardInterruptible(&dummy, Foreach_run_for_state_command(), c, COMMAND_LOCKED, o, Foreach_continue_unplanned_helper());
        }
    }
    
    static void planner_aborted_handler (Context c)
    {
        PrinterMain *o = self(c);
        auto *ob = Object::self(c);
        o->debugAccess(c);
        AMBRO_ASSERT(ob->planner_state == PLANNER_PROBE)
        
        TupleForEachForward(&o->m_axes, Foreach_fix_aborted_pos(), c);
        o->m_transform_feature.do_pending_virt_update(c);
        o->m_probe_feature.custom_aborted_handler(c);
    }
    
    static void planner_underrun_callback (Context c)
    {
        auto *ob = Object::self(c);
        ob->underrun_count++;
    }
    
    static void planner_channel_callback (typename ThePlanner::template Channel<0>::CallbackContext c, PlannerChannelPayload *payload)
    {
        PrinterMain *o = self(c);
        o->debugAccess(c);
        
        TupleForOneBoolOffset<0>(payload->type, &o->m_heaters, Foreach_channel_callback(), c, &payload->heaters) ||
        TupleForOneBoolOffset<TypeListLength<HeatersList>::value>(payload->type, &o->m_fans, Foreach_channel_callback(), c, &payload->fans);
    }
    
    template <int AxisIndex>
    static bool planner_prestep_callback (typename ThePlanner::template Axis<AxisIndex>::StepperCommandCallbackContext c)
    {
        PrinterMain *o = self(c);
        return o->m_probe_feature.prestep_callback(c);
    }
    
    struct MoveBuildState {
        bool seen_cartesian;
    };
    
    static void move_begin (Context c, MoveBuildState *s)
    {
        PrinterMain *o = self(c);
        PhysVirtAxisHelperTuple dummy;
        TupleForEachForward(&dummy, Foreach_init_new_pos(), c);
        s->seen_cartesian = false;
    }
    
    template <int PhysVirtAxisIndex>
    static void move_add_axis (Context c, MoveBuildState *s, FpType value)
    {
        PhysVirtAxisHelper<PhysVirtAxisIndex>::update_new_pos(c, s, value);
    }
    
    struct ReqPosSrc {
        Context m_c;
        template <int Index>
        FpType get () { return Axis<Index>::self(m_c)->m_req_pos; }
    };
    
    static void move_end (Context c, MoveBuildState *s, FpType time_freq_by_max_speed)
    {
        PrinterMain *o = self(c);
        auto *ob = Object::self(c);
        AMBRO_ASSERT(ob->planner_state == PLANNER_RUNNING || ob->planner_state == PLANNER_PROBE)
        AMBRO_ASSERT(o->m_planning_pull_pending)
        AMBRO_ASSERT(FloatIsPosOrPosZero(time_freq_by_max_speed))
        
        if (o->m_transform_feature.is_splitting(c)) {
            o->m_transform_feature.handle_virt_move(c, time_freq_by_max_speed);
            return;
        }
        PlannerSplitBuffer *cmd = o->m_planner.getBuffer(c);
        FpType distance_squared = 0.0f;
        FpType total_steps = 0.0f;
        TupleForEachForward(&o->m_axes, Foreach_do_move(), c, ReqPosSrc{c}, WrapBool<true>(), &distance_squared, &total_steps, cmd);
        o->m_transform_feature.do_pending_virt_update(c);
        if (total_steps != 0.0f) {
            cmd->rel_max_v_rec = total_steps * (FpType)(1.0 / (Params::MaxStepsPerCycle::value() * F_CPU * Clock::time_unit));
            if (s->seen_cartesian) {
                cmd->rel_max_v_rec = FloatMax(cmd->rel_max_v_rec, FloatSqrt(distance_squared) * time_freq_by_max_speed);
            } else {
                TupleForEachForward(&o->m_axes, Foreach_limit_axis_move_speed(), c, time_freq_by_max_speed, cmd);
            }
            o->m_planner.axesCommandDone(c);
        } else {
            o->m_planner.emptyDone(c);
        }
        submitted_planner_command(c);
    }
    
    static void submitted_planner_command (Context c)
    {
        PrinterMain *o = self(c);
        auto *ob = Object::self(c);
        AMBRO_ASSERT(ob->planner_state != PLANNER_NONE)
        AMBRO_ASSERT(o->m_planning_pull_pending)
        
        o->m_planning_pull_pending = false;
        ob->force_timer.unset(c);
    }
    
    static void custom_planner_init (Context c, uint8_t type, bool enable_prestep_callback)
    {
        PrinterMain *o = self(c);
        auto *ob = Object::self(c);
        AMBRO_ASSERT(ob->locked)
        AMBRO_ASSERT(ob->planner_state == PLANNER_NONE)
        AMBRO_ASSERT(type == PLANNER_PROBE)
        
        ob->planner_state = type;
        o->m_planner.init(c, enable_prestep_callback);
        o->m_planning_pull_pending = false;
        now_active(c);
    }
    
    static void custom_planner_deinit (Context c)
    {
        PrinterMain *o = self(c);
        auto *ob = Object::self(c);
        AMBRO_ASSERT(ob->locked)
        AMBRO_ASSERT(ob->planner_state == PLANNER_PROBE)
        
        o->m_planner.deinit(c);
        ob->planner_state = PLANNER_NONE;
        now_inactive(c);
    }
    
    static void custom_planner_wait_finished (Context c)
    {
        PrinterMain *o = self(c);
        auto *ob = Object::self(c);
        AMBRO_ASSERT(ob->locked)
        AMBRO_ASSERT(ob->planner_state == PLANNER_PROBE)
        AMBRO_ASSERT(o->m_planning_pull_pending)
        
        o->m_planner.waitFinished(c);
    }
    
    Object m_object;
    TheWatchdog m_watchdog;
    SerialFeature m_serial_feature;
    SdCardFeature m_sdcard_feature;
    AxesTuple m_axes;
    TransformFeature m_transform_feature;
    HeatersTuple m_heaters;
    FansTuple m_fans;
    ProbeFeature m_probe_feature;
    CurrentFeature m_current_feature;
    union {
        struct {
            HomingStateTuple m_homers;
            AxisCountType m_homing_rem_axes;
        };
        struct {
            ThePlanner m_planner;
            bool m_planning_pull_pending;
        };
    };
    
    struct WatchdogPosition : public MemberPosition<Position, TheWatchdog, &PrinterMain::m_watchdog> {};
    struct BlinkerPosition : public MemberPosition<Position, TheBlinker, &PrinterMain::m_blinker> {};
    template <int AxisIndex> struct AxisPosition : public TuplePosition<Position, AxesTuple, &PrinterMain::m_axes, AxisIndex> {};
    template <int AxisIndex> struct HomingFeaturePosition : public MemberPosition<AxisPosition<AxisIndex>, typename Axis<AxisIndex>::HomingFeature, &Axis<AxisIndex>::m_homing_feature> {};
    template <int AxisIndex> struct HomingStatePosition : public TuplePosition<Position, HomingStateTuple, &PrinterMain::m_homers, AxisIndex> {};
    struct TransformFeaturePosition : public MemberPosition<Position, TransformFeature, &PrinterMain::m_transform_feature> {};
    struct SerialFeaturePosition : public MemberPosition<Position, SerialFeature, &PrinterMain::m_serial_feature> {};
    struct SdCardFeaturePosition : public MemberPosition<Position, SdCardFeature, &PrinterMain::m_sdcard_feature> {};
    struct PlannerPosition : public MemberPosition<Position, ThePlanner, &PrinterMain::m_planner> {};
    template <int HeaterIndex> struct HeaterPosition : public TuplePosition<Position, HeatersTuple, &PrinterMain::m_heaters, HeaterIndex> {};
    template <int FanIndex> struct FanPosition : public TuplePosition<Position, FansTuple, &PrinterMain::m_fans, FanIndex> {};
    struct ProbeFeaturePosition : public MemberPosition<Position, ProbeFeature, &PrinterMain::m_probe_feature> {};
    struct CurrentFeaturePosition : public MemberPosition<Position, CurrentFeature, &PrinterMain::m_current_feature> {};
    
    struct BlinkerHandler : public AMBRO_WFUNC_TD(&PrinterMain::blinker_handler) {};
    template <int AxisIndex> struct PlannerGetAxisStepper : public AMBRO_WFUNC_TD(&PrinterMain::template planner_get_axis_stepper<AxisIndex>) {};
    struct PlannerPullHandler : public AMBRO_WFUNC_TD(&PrinterMain::planner_pull_handler) {};
    struct PlannerFinishedHandler : public AMBRO_WFUNC_TD(&PrinterMain::planner_finished_handler) {};
    struct PlannerAbortedHandler : public AMBRO_WFUNC_TD(&PrinterMain::planner_aborted_handler) {};
    struct PlannerUnderrunCallback : public AMBRO_WFUNC_TD(&PrinterMain::planner_underrun_callback) {};
    struct PlannerChannelCallback : public AMBRO_WFUNC_TD(&PrinterMain::planner_channel_callback) {};
    template <int AxisIndex> struct PlannerPrestepCallback : public AMBRO_WFUNC_TD(&PrinterMain::template planner_prestep_callback<AxisIndex>) {};
    template <int AxisIndex> struct AxisStepperConsumersList {
        using List = JoinTypeLists<
            MakeTypeList<typename ThePlanner::template TheAxisStepperConsumer<AxisIndex>>,
            typename Axis<AxisIndex>::HomingFeature::template MakeAxisStepperConsumersList<typename Axis<AxisIndex>::HomingFeature>
        >;
    };
    
public:
    struct Object : public ObjBase<PrinterMain, void, MakeTypeList<
        TheBlinker,
        TheSteppers
    >> {
        static Object * self (Context c)
        {
            PrinterMain *o = PrinterMain::self(c);
            return &o->m_object;
        }
        
        typename Loop::QueuedEvent unlocked_timer;
        typename Loop::QueuedEvent disable_timer;
        typename Loop::QueuedEvent force_timer;
        TimeType inactive_time;
        TimeType last_active_time;
        FpType time_freq_by_max_speed;
        uint32_t underrun_count;
        bool locked;
        uint8_t planner_state;
    };
};

#include <aprinter/EndNamespace.h>

#endif
