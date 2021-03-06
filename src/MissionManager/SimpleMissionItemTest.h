/****************************************************************************
 *
 *   (c) 2009-2016 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#pragma once

#include "VisualMissionItemTest.h"
#include "SimpleMissionItem.h"

/// Unit test for SimpleMissionItem
class SimpleMissionItemTest : public VisualMissionItemTest
{
    Q_OBJECT
    
public:
    SimpleMissionItemTest(void);

    void init(void) override;
    void cleanup(void) override;

private slots:
    void _testSignals(void);
    void _testCameraSectionSignals(void);
    void _testEditorFacts(void);
    void _testDefaultValues(void);
    
private:
    enum {
        commandChangedIndex = 0,
        frameChangedIndex,
        friendlyEditAllowedChangedIndex,
        headingDegreesChangedIndex,
        rawEditChangedIndex,
        cameraSectionChangedIndex,
        speedSectionChangedIndex,
        maxSignalIndex,
    };

    enum {
        commandChangedMask =                1 << commandChangedIndex,
        frameChangedMask =                  1 << frameChangedIndex,
        friendlyEditAllowedChangedMask =    1 << friendlyEditAllowedChangedIndex,
        headingDegreesChangedMask =         1 << headingDegreesChangedIndex,
        rawEditChangedMask =                1 << rawEditChangedIndex,
        cameraSectionChangedMask =          1 << cameraSectionChangedIndex,
        speedSectionChangedMask =           1 << speedSectionChangedIndex
    };

    static const size_t cSimpleItemSignals = maxSignalIndex;
    const char*         rgSimpleItemSignals[cSimpleItemSignals];

    typedef struct {
        MAV_CMD        command;
        MAV_FRAME      frame;
    } ItemInfo_t;
    
    typedef struct {
        const char* name;
        double      value;
    } FactValue_t;
    
    typedef struct {
        size_t              cFactValues;
        const FactValue_t*  rgFactValues;
        bool                relativeAltCheckbox;
    } ItemExpected_t;

    SimpleMissionItem*  _simpleItem;
    MultiSignalSpy*     _spySimpleItem;
    MultiSignalSpy*     _spyVisualItem;

    static const ItemInfo_t     _rgItemInfo[];
    static const ItemExpected_t _rgItemExpected[];
    static const FactValue_t    _rgFactValuesWaypoint[];
    static const FactValue_t    _rgFactValuesLoiterUnlim[];
    static const FactValue_t    _rgFactValuesLoiterTurns[];
    static const FactValue_t    _rgFactValuesLoiterTime[];
    static const FactValue_t    _rgFactValuesLand[];
    static const FactValue_t    _rgFactValuesTakeoff[];
    static const FactValue_t    _rgFactValuesConditionDelay[];
    static const FactValue_t    _rgFactValuesDoJump[];
};
