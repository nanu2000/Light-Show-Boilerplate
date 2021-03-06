#ifndef DISPLAY_STATISTICS_H
#define DISPLAY_STATISTICS_H
#include "Component.h"
#include "GUIResizingInfo.h"
#include "GuiString.h"
#include "LTime.h"
#include <numeric>

class DisplayStatistics : public Component<DisplayStatistics> {

public:
    void initialize(TextMap& textMap, Texture& texture) {
        guiString.initialize(textMap, texture);
    }

private:
    GuiString guiString = GuiString(20);

    float lastUnit = 0;

    float currentInterval   = 0.f;
    std::string currentFPS  = "0";
    std::string currentMSPF = "0";
    int lastFPS             = 0;

    friend class DisplayStatisticsSystem;
};

#endif
