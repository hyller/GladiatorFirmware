/**
* @file
* @brief Public QF-nano interface.
* @ingroup qfn
* @cond
******************************************************************************
* Last updated for version 5.4.2
* Last updated on  2015-06-12
*
*                    Q u a n t u m     L e a P s
*                    ---------------------------
*                    innovating embedded systems
*
* Copyright (C) Quantum Leaps, www.state-machine.com.
*
* This program is open source software: you can redistribute it and/or
* modify it under the terms of the GNU General Public License as published
* by the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* Alternatively, this program may be distributed and modified under the
* terms of Quantum Leaps commercial licenses, which expressly supersede
* the GNU General Public License and are specifically designed for
* licensees interested in retaining the proprietary status of their code.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program. If not, see <http://www.gnu.org/licenses/>.
*
* Contact information:
* Web:   www.state-machine.com
* Email: info@state-machine.com
******************************************************************************
* @endcond
*/
#ifndef qfn_h
#define qfn_h

/**
* @description
* This header file must be included in all modules that use QP-nano.
* Typically, this header file is included indirectly through the
* header file qpn_port.h.
*/

/****************************************************************************/
#if (QF_MAX_ACTIVE < 1) || (8 < QF_MAX_ACTIVE)
    #error "QF_MAX_ACTIVE not defined or out of range. Valid range is 1..8"
#endif

/****************************************************************************/
#ifndef QF_TIMEEVT_CTR_SIZE
    /*! macro to override the default QTimeEvtCtr size.
    * Valid values 0, 1, 2, or 4; default 0
    */
    #define QF_TIMEEVT_CTR_SIZE 0
#endif
#if (QF_TIMEEVT_CTR_SIZE == 0)
    /* no time events */
#elif (QF_TIMEEVT_CTR_SIZE == 1)
    typedef uint_fast8_t QTimeEvtCtr;
#elif (QF_TIMEEVT_CTR_SIZE == 2)
    /*! type of the Time Event counter, which determines the dynamic
    * range of the time delays measured in clock ticks.
    */
    /**
    * @description
    * This typedef is configurable via the preprocessor switch
    * #QF_TIMEEVT_CTR_SIZE. The other possible values of this type are
    * as follows: @n
    * none when (QF_TIMEEVT_CTR_SIZE not defined or == 0), @n
    * uint_fast8_t  when (QF_TIMEEVT_CTR_SIZE == 1); @n
    * uint_fast16_t when (QF_TIMEEVT_CTR_SIZE == 2); and @n
    * uint_fast32_t when (QF_TIMEEVT_CTR_SIZE == 4).
    */
    typedef uint_fast16_t QTimeEvtCtr;
#elif (QF_TIMEEVT_CTR_SIZE == 4)
    typedef uint_fast32_t QTimeEvtCtr;
#else
    #error "QF_TIMER_SIZE defined incorrectly, expected 1, 2, or 4"
#endif

#if (QF_TIMEEVT_CTR_SIZE != 0)
    /*! Timer structure the active objects */
    typedef struct {
        QTimeEvtCtr nTicks;   /*!< timer tick counter */
#ifdef QF_TIMEEVT_PERIODIC
        QTimeEvtCtr interval; /*!< timer interval */
#endif /* QF_TIMEEVT_PERIODIC */
    } QTimer;
#endif /* (QF_TIMEEVT_CTR_SIZE != 0) */

#ifndef QF_MAX_TICK_RATE
    /*! Default value of the macro configurable value in qpn_port.h */
    #define QF_MAX_TICK_RATE     1
#elif (QF_MAX_TICK_RATE > 4)
    #error "QF_MAX_TICK_RATE exceeds the 4 limit"
#endif

/****************************************************************************/
/*! QMActive active object (based on QMsm-implementation) */
/**
* @description
* QMActive is the base structure for derivation of active objects. Active
* objects in QF-nano are encapsulated tasks (each embedding a state machine
* and an event queue) that communicate with one another asynchronously by
* sending and receiving events. Within an active object, events are
* processed sequentially in a run-to-completion (RTC) fashion, while QF
* encapsulates all the details of thread-safe event exchange and queuing.
*
* @note ::QMActive is not intended to be instantiated directly, but rather
* serves as the base structure for derivation of active objects in the
* application code.
*
* @sa ::QActive for the description of the data members @n @ref oop
*
* @usage
* The following example illustrates how to derive an active object from
* ::QMActive. Please note that the ::QMActive member super_ is defined as
* the __first__ member of the derived struct.
* @include qfn_qmactive.c
*/
typedef struct {
    QMsm super; /**< derives from the ::QMsm base class */

#if (QF_TIMEEVT_CTR_SIZE != 0)
    /*! Timer for the active object */
    QTimer tickCtr[QF_MAX_TICK_RATE];
#endif /* (QF_TIMEEVT_CTR_SIZE != 0) */

    /*! priority of the active object (1..QF_MAX_ACTIVE) */
    uint_fast8_t prio;

    /*! offset to where next event will be inserted into the buffer */
    uint_fast8_t volatile head;

    /*! offset of where next event will be extracted from the buffer */
    uint_fast8_t volatile tail;

    /*! number of events currently present in the queue
    * (events in the ring buffer + 1 event in the state machine)
    */
    uint_fast8_t volatile nUsed;

} QMActive;

/*! Virtual table for the QMActive class */
typedef struct {
    QMsmVtbl super; /*!< inherits QMsmVtbl */

#if (Q_PARAM_SIZE != 0)
    /*! virtual function to asynchronously post (FIFO) an event to an AO
    * (task context).
    */
    /** @sa QACTIVE_POST() and QACTIVE_POST_X() */
    bool (*post)(QMActive * const me, uint_fast8_t const margin,
                 enum_t const sig, QParam const par);

    /*! virtual function to asynchronously post (FIFO) an event to an AO
    * (ISR context).
    */
    /** @sa QACTIVE_POST_ISR() and QACTIVE_POST_X_ISR() */
    bool (*postISR)(QMActive * const me, uint_fast8_t const margin,
                    enum_t const sig, QParam const par);
#else
    bool (*post)   (QMActive * const me, uint_fast8_t const margin,
                    enum_t const sig);
    bool (*postISR)(QMActive * const me, uint_fast8_t const margin,
                    enum_t const sig);
#endif
} QMActiveVtbl;


#ifndef Q_NMSM

/*! QMActive constructor */
/**
* @description
* @param[in,out] me      pointer the active object (see @ref oop)
* @param[in]     initial is the pointer to the top-most initial transition
*
* @note Must be called exactly ONCE for each active object
* in the application before calling QF_run().
*/
void QMActive_ctor(QMActive * const me, QStateHandler initial);

#endif /* Q_NMSM */


/****************************************************************************/
/*! Active Object (based on QHsm-implementation) */
/**
* @description
* Active objects in QP are encapsulated state machines (each embedding an
* event queue and a thread) that communicate with one another asynchronously
* by sending and receiving events. Within an active object, events are
* processed sequentially in a run-to-completion (RTC) fashion, while QF
* encapsulates all the details of thread-safe event exchange and queuing.
* @n@n
* QActive represents an active object that uses the QHsm-style
* implementation strategy for state machines. This strategy is tailored
* to manual coding, but it is also supported by the QM modeling tool.
* The resulting code is slower than in the QMsm-style implementation
* strategy.
*
* @note
* ::QActive inherits ::QMActive exactly, without adding any new attributes
* (or operations) and therefore, ::QActive is typedef'ed as ::QMActive.
* ::QActive is not intended to be instantiated directly, but rather serves
* as the base class for derivation of active objects in the application.
*
* @sa ::QMActive
*
* @usage
* The following example illustrates how to derive an active object from
* QActive. Please note that the QActive member @c super is defined as the
* __first__ member of the derived struct (see @ref oop).
* @include qfn_qactive.c
*/
typedef QMActive QActive;

/*! Virtual Table for the ::QActive class (inherited from ::QMActiveVtbl */
/**
* @note
* ::QActive inherits ::QMActive exactly, without adding any new virtual
* functions and therefore, ::QActiveVtbl is typedef'ed as ::QMActiveVtbl.
*/
typedef QMActiveVtbl QActiveVtbl;


#ifndef Q_NHSM

/*! protected "constructor" of an QActive active object. */
void QActive_ctor(QActive * const me, QStateHandler initial);

#endif /* Q_NHSM */

#if (Q_PARAM_SIZE != 0)
    /*! Polymorphically posts an event to an active object (FIFO)
    * with delivery guarantee (task context).
    */
    /**
    * @description
    * This macro asserts if the queue overflows and cannot accept the event.
    *
    * @param[in,out] me_   pointer (see @ref oop)
    * @param[in]     sig_  signal of the event to post
    * @param[in]     par_  parameter of the event to post.
    *
    * @sa QACTIVE_POST_X(), QActive_postX_(),
    * QACTIVE_POST_ISR(), QActive_postXISR_().
    *
    * @usage
    * @include qfn_post.c
    */
    #define QACTIVE_POST(me_, sig_, par_) \
        ((void)(*((QMActiveVtbl const *)( \
            QF_ACTIVE_CAST((me_))->super.vptr))->post)( \
                QF_ACTIVE_CAST((me_)), (uint_fast8_t)0, \
                (enum_t)(sig_), (QParam)(par_)))

    /*! Polymorphically posts an event to an active object (FIFO)
    * without delivery guarantee (task context).
    */
    /**
    * @description
    * This macro does not assert if the queue overflows and cannot accept
    * the event with the specified margin of free slots remaining.
    *
    * @param[in,out] me_     pointer (see @ref oop)
    * @param[in]     margin_ the minimum free slots in the queue, which
    *                        must still be available after posting the event
    * @param[in]     sig_    signal of the event to post
    * @param[in]     par_    parameter of the event to post.
    *
    * @returns 'true' if the posting succeeded, and 'false' if the posting
    * failed due to insufficient margin of free slots available in the queue.
    *
    * @usage
    * @include qfn_postx.c
    */
    #define QACTIVE_POST_X(me_, margin_, sig_, par_) \
        ((*((QMActiveVtbl const *)( \
            QF_ACTIVE_CAST((me_))->super.vptr))->post)( \
                QF_ACTIVE_CAST((me_)), \
                (margin_), (enum_t)(sig_), (QParam)(par_)))

    /*! Polymorphically posts an event to an active object (FIFO)
    * with delivery guarantee (ISR context).
    */
    /**
    * @description
    * This macro asserts if the queue overflows and cannot accept the event.
    *
    * @param[in,out] me_   pointer (see @ref oop)
    * @param[in]     sig_  signal of the event to post
    * @param[in]     par_  parameter of the event to post.
    *
    * @sa QACTIVE_POST_X(), QActive_postX_().
    *
    * @usage
    * @include qfn_post.c
    */
    #define QACTIVE_POST_ISR(me_, sig_, par_) \
        ((void)(*((QMActiveVtbl const *)( \
            QF_ACTIVE_CAST((me_))->super.vptr))->postISR)( \
                QF_ACTIVE_CAST((me_)), (uint_fast8_t)0, \
                (enum_t)(sig_), (QParam)(par_)))

    /*! Polymorphically posts an event to an active object (FIFO)
    * without delivery guarantee (ISR context).
    */
    /**
    * @description
    * This macro does not assert if the queue overflows and cannot accept
    * the event with the specified margin of free slots remaining.
    *
    * @param[in,out] me_     pointer (see @ref oop)
    * @param[in]     margin_ the minimum free slots in the queue, which
    *                        must still be available after posting the event
    * @param[in]     sig_    signal of the event to post
    * @param[in]     par_    parameter of the event to post.
    *
    * @returns 'true' if the posting succeeded, and 'false' if the posting
    * failed due to insufficient margin of free slots available in the queue.
    *
    * @usage
    * @include qfn_postx.c
    */
    #define QACTIVE_POST_X_ISR(me_, margin_, sig_, par_) \
        ((*((QMActiveVtbl const *)( \
            QF_ACTIVE_CAST((me_))->super.vptr))->postISR)( \
                QF_ACTIVE_CAST((me_)), (margin_), \
                (enum_t)(sig_), (QParam)(par_)))

    /*! Implementation of the task-level event posting */
    bool QActive_postX_(QMActive * const me, uint_fast8_t margin,
                        enum_t const sig, QParam const par);

    /*! Implementation of the ISR-level event posting */
    bool QActive_postXISR_(QMActive * const me, uint_fast8_t margin,
                           enum_t const sig, QParam const par);

#else /* no event parameter */
    #define QACTIVE_POST(me_, sig_) \
        ((void)(*((QMActiveVtbl const *)( \
            QF_ACTIVE_CAST((me_))->super.vptr))->post)( \
                QF_ACTIVE_CAST((me_)), (uint_fast8_t)0, (enum_t)(sig_)))

    #define QACTIVE_POST_X(me_, margin_, sig_) \
        ((*((QMActiveVtbl const *)((me_)->super.vptr))->post)((me_), \
                                   (margin_), (sig_)))

    bool QActive_postX_(QMActive * const me, uint_fast8_t margin,
                        enum_t const sig);

    #define QACTIVE_POST_ISR(me_, sig_) \
        ((void)(*((QActiveVtbl const*)( \
            QF_ACTIVE_CAST((me_))->super.vptr))->postISR)( \
                 QF_ACTIVE_CAST((me_)), (uint_fast8_t)0, (enum_t)(sig_)))

    #define QACTIVE_POST_X_ISR(me_, margin_, sig_) \
        ((*((QActiveVtbl const *)( \
            QF_ACTIVE_CAST((me_))->super.vptr))->postISR)( \
                QF_ACTIVE_CAST((me_)), (margin_), (enum_t)(sig_)))

    bool QActive_postXISR_(QMActive * const me, uint_fast8_t margin,
                           enum_t const sig);
#endif

#if (QF_TIMEEVT_CTR_SIZE != 0)

    /*! Processes all armed time events at every clock tick. */
    void QF_tickXISR(uint_fast8_t const tickRate);

#ifdef QF_TIMEEVT_PERIODIC
    /*! Arm the QP-nano one-shot time event. */
    void QActive_armX(QMActive * const me, uint_fast8_t const tickRate,
                      QTimeEvtCtr const nTicks, QTimeEvtCtr const interval);
#else
    /*! Arm the QP-nano one-shot time event. */
    void QActive_armX(QActive * const me, uint_fast8_t const tickRate,
                      QTimeEvtCtr const nTicks);
#endif

    /*! Disarm a time event. Since the tick counter */
    void QActive_disarmX(QMActive * const me, uint_fast8_t const tickRate);

#endif /* (QF_TIMEEVT_CTR_SIZE != 0) */


/****************************************************************************/
/* QF-nano protected methods ...*/

/*! QF-nano initialization. */
/*
* @note Function QF_init() is defined in the separate module qfn_init.c,
* which needs to be included in the build only if the non-standard
* initialization is required.
*/
void QF_init(void);

/*! QF-nano termination. */
/**
* @description
* This function terminates QF and performs any necessary cleanup.
* In QF-nano this function is defined in the BSP. Many QF ports might not
* require implementing QF_stop() at all, because many embedded applications
* don't have anything to exit to.
*/
void QF_stop(void);

/*! Startup QF-nano callback. */
/**
* @description
* The time line for calling QF_onStartup() depends on the particular
* QF port. In most cases, QF_onStartup() is called from QF_run(), right
* before starting any multitasking kernel or the background loop.
*
* @sa QF initialization example for ::QMActiveCB.
*/
void QF_onStartup(void);

/*! Transfers control to QF-nano to run the application. */
int_t QF_run(void);


/****************************************************************************/
/*! QMActive Control Block
*
* QMActiveCB represents the read-only information that the QF-nano needs to
* manage the active object. QMActiveCB objects are grouped in the array
* QF_active[], which typically can be placed in ROM.
*
* @usage
* The following example illustrates how to allocate and initialize the
* ::QMActive control blocks in the array QF_active[].
* @include qfn_main.c
*/
typedef struct {
    QMActive *act;   /*!< pointer to the active object structure */
    QEvt     *queue; /*!< pointer to the event queue buffer */
    uint8_t   qlen;  /*!< the length of the queue ring buffer */
} QMActiveCB;

/** active object control blocks */
/*lint -save -e960    MISRA-C:2004 8.12, extern array declared without size */
extern QMActiveCB const Q_ROM QF_active[];
/*lint -restore */

/*! Ready set of QF-nano. */
extern uint_fast8_t volatile QF_readySet_;

#ifndef QF_LOG2

    /*! Lookup table for (log2(n) + 1), where n is the index into the table.
    * This lookup delivers the 1-based number of the most significant 1-bit
    * of a nibble.
    */
    extern uint8_t const Q_ROM QF_log2Lkup[16];

#endif


#ifdef QF_TIMEEVT_USAGE

    /*! Timer set of QF-nano. */
    extern uint_fast8_t volatile QF_timerSetX_[QF_MAX_TICK_RATE];

#endif  /* QF_TIMEEVT_USAGE */


/*! Lookup table for ~(1 << (n - 1)), where n is the index into the table. */
extern uint8_t const Q_ROM QF_invPow2Lkup[9];


/****************************************************************************/
/*! This macro encapsulates accessing the active object queue at a
* given index, which violates MISRA-C 2004 rules 17.4(req) and 11.4(adv).
* This macro helps to localize this deviation.
*/
#define QF_ROM_QUEUE_AT_(ao_, i_) (((QEvt *)Q_ROM_PTR((ao_)->queue))[(i_)])

/*! This macro encapsulates accessing the active object control block,
* which violates MISRA-C 2004 rule 11.4(adv). This macro helps to localize
* this deviation.
*/
#define QF_ROM_ACTIVE_GET_(p_) ((QMActive *)Q_ROM_PTR(QF_active[(p_)].act))

/*! This macro encapsulates the upcast to QMActive*
*
* This macro encapsulates up-casting a pointer to a subclass of ::QMActive
* to the base class ::QMActive, which violates MISRA-C 2004 rule 11.4(adv).
* This macro helps to localize this deviation.
*/
#define QF_ACTIVE_CAST(a_)     ((QMActive *)(a_))

#endif /* qfn_h */

