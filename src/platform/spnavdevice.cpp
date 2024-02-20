//-----------------------------------------------------------------------------
// Cross-platform handling of spnavdev 6-dof input.
//
// Copyright 2021 Collabora, Ltd.
//-----------------------------------------------------------------------------

#include "spnavdevice.h"
#include "config.h"

#ifdef HAVE_SPNAVDEV
#    include "spnavdev.h"
#    include <string>
#    include <solvespace.h>

NavDeviceWrapper::NavDeviceWrapper(const char* device) {
    dev = spndev_open(device);
    if(dev == nullptr) {
        return;
    }

	int fd = spndev_fd(dev);
    populateAxes();
    populateButtons();
}
NavDeviceWrapper::~NavDeviceWrapper() {
    if(dev != nullptr) {
        spndev_close(dev);
        dev = nullptr;
    }
}

static double transformIndex(union spndev_event const& ev,
                             NavDeviceWrapper::AxisData const& axisData) {
    ssassert(ev.type == SPNDEV_MOTION, "shouldn't be in here if not a motion event");
    if(axisData.spnavdevIndex < 0) {
        return 0;
    }
    return double(ev.mot.v[axisData.spnavdevIndex])*axisData.scale;
}
bool NavDeviceWrapper::process(SolveSpace::Platform::SixDofEvent& event) {
    using SolveSpace::Platform::SixDofEvent;
    union spndev_event ev = {};
    if(0 == spndev_process(dev, &ev)) {
        return false;
    }
    if(ctrlPressed) {
        event.controlDown = true;
    }
    if(shiftPressed) {
        event.shiftDown = true;
    }
    switch(ev.type) {
    case SPNDEV_MOTION:
        event.type         = SixDofEvent::Type::MOTION;
        event.translationX = transformIndex(ev, axes[0]);
        event.translationY = transformIndex(ev, axes[1]);
        event.translationZ = transformIndex(ev, axes[2]);
        event.rotationX    = transformIndex(ev, axes[3]);
        event.rotationY    = transformIndex(ev, axes[4]);
        event.rotationZ    = transformIndex(ev, axes[5]);
        return true;
    case SPNDEV_BUTTON: {
        if(ev.bn.num >= buttons.size()) {
            return false;
        }
        const NavButton meaning = buttons[ev.bn.num];
        auto type = ev.bn.press ? SixDofEvent::Type::PRESS : SixDofEvent::Type::RELEASE;
        switch(meaning) {
        case NavButton::UNUSED:
            // we don't handle this button.
            return false;

        case NavButton::SHIFT:
            // handled internally to this class
            shiftPressed = type == SixDofEvent::Type::PRESS;
            return false;

        case NavButton::CTRL:
            // handled internally to this class
            ctrlPressed = type == SixDofEvent::Type::PRESS;
            return false;

        case NavButton::FIT:
            event.button = SixDofEvent::Button::FIT;
            event.type   = type;
            return true;
        }
        break;
    }
    default: return false;
    }
    return false;
}

void NavDeviceWrapper::populateAxes() {
    // This array remaps the axis and their directions/scaling, so that they match what the standard
    // 3Dconnexion driver outputs. In this way SolveSpace will control the object "properly". The
    // array may not be requiered if it turns out that all devices use the same axis orientation and
    // have similar sensitivities, but it is better if we test them. The ones that are already
    // tested and working are marked.
    // See also: https://github.com/FreeSpacenav/spacenavd/blob/master/src/dev.c#L38
    const struct {
        const std::wstring name;
        const std::string axis_names[6];
        const double axis_scale[6];
    } axisRemap[] = {
        {L"SpaceMouse Plus XT USB" ,          {"Tx", "Tz", "Ty", "Rx", "Rz", "Ry"}, {1.0, 1.0, -1.0, .001, .001, -.001}},
        {L"CadMan USB",                       {"Tx", "Tz", "Ty", "Rx", "Rz", "Ry"}, {1.0, 1.0, -1.0, .001, .001, -.001}},   // Tested
        {L"SpaceMouse Classic USB",           {"Tx", "Tz", "Ty", "Rx", "Rz", "Ry"}, {1.0, 1.0, -1.0, .001, .001, -.001}},
        {L"SpaceBall 5000 USB",               {"Tx", "Tz", "Ty", "Rx", "Rz", "Ry"}, {1.0, 1.0, -1.0, .001, .001, -.001}},   // Tested
        {L"SpaceTraveler USB",                {"Tx", "Tz", "Ty", "Rx", "Rz", "Ry"}, {1.0, 1.0, -1.0, .001, .001, -.001}},   // Tested
        {L"SpacePilot",                       {"Tx", "Tz", "Ty", "Rx", "Rz", "Ry"}, {1.0, 1.0, -1.0, .001, .001, -.001}},   // Tested
        {L"SpaceNavigator",                   {"Tx", "Tz", "Ty", "Rx", "Rz", "Ry"}, {1.0, 1.0, -1.0, .001, .001, -.001}},   // Tested
        {L"SpaceExplorer",                    {"Tx", "Tz", "Ty", "Rx", "Rz", "Ry"}, {1.0, 1.0, -1.0, .001, .001, -.001}},   // Tested
        {L"SpaceNavigator for Notebooks",     {"Tx", "Tz", "Ty", "Rx", "Rz", "Ry"}, {1.0, 1.0, -1.0, .001, .001, -.001}},
        {L"SpacePilot Pro",                   {"Tx", "Tz", "Ty", "Rx", "Rz", "Ry"}, {1.0, 1.0, -1.0, .001, .001, -.001}},
        {L"SpaceMouse Pro",                   {"Tx", "Tz", "Ty", "Rx", "Rz", "Ry"}, {1.0, 1.0, -1.0, .001, .001, -.001}},
        {L"NuLOOQ",                           {"Tx", "Tz", "Ty", "Rx", "Rz", "Ry"}, {1.0, 1.0, -1.0, .001, .001, -.001}},
        {L"LIPARI",                           {"Tx", "Tz", "Ty", "Rx", "Rz", "Ry"}, {1.0, 1.0, -1.0, .001, .001, -.001}},
        {L"SpaceMouse Wireless (cabled)",     {"Tx", "Tz", "Ty", "Rx", "Rz", "Ry"}, {1.0, 1.0, -1.0, .001, .001, -.001}},
        {L"SpaceMouse Wireless Receiver",     {"Tx", "Tz", "Ty", "Rx", "Rz", "Ry"}, {1.0, 1.0, -1.0, .001, .001, -.001}},
        {L"SpaceMouse Pro Wireless (cabled)", {"Tx", "Tz", "Ty", "Rx", "Rz", "Ry"}, {1.0, 1.0, -1.0, .001, .001, -.001}},
        {L"SpaceMouse Pro Wireless Receiver", {"Tx", "Tz", "Ty", "Rx", "Rz", "Ry"}, {1.0, 1.0, -1.0, .001, .001, -.001}},
        {L"SpaceMouse Enterprise",            {"Tx", "Tz", "Ty", "Rx", "Rz", "Ry"}, {1.0, 1.0, -1.0, .001, .001, -.001}},
        {L"SpaceMouse Compact",               {"Tx", "Tz", "Ty", "Rx", "Rz", "Ry"}, {1.0, 1.0, -1.0, .001, .001, -.001}},
        {L"SpaceMouse Module",                {"Tx", "Tz", "Ty", "Rx", "Rz", "Ry"}, {1.0, 1.0, -1.0, .001, .001, -.001}},
        {L"SpaceMouse Universal Receiver",    {"Tx", "Tz", "Ty", "Rx", "Rz", "Ry"}, {1.0, 1.0, -1.0, .001, .001, -.001}},
    };

    using std::begin;
    using std::end;

    auto da = std::find_if(begin(axisRemap), end(axisRemap),
                           [&](auto const& devaxis) { return devaxis.name == (wchar_t *)spndev_name(dev); });

    if(da != end(axisRemap)) {
        const auto b        = begin(da->axis_names);
        const auto e        = end(da->axis_names);
        const auto num_axes = spndev_num_axes(dev);
        for(int axis_idx = 0; axis_idx < num_axes; ++axis_idx) {
            auto axis_name = spndev_axis_name(dev, axis_idx);
            auto it = std::find_if(b, e, [&](std::string const& name) { return name == axis_name; });
            if(it != e) {
                ptrdiff_t remapped_index = std::distance(b, it);
                axes[remapped_index]     = AxisData{axis_idx, da->axis_scale[axis_idx]};
            }
        }
    }
}

void NavDeviceWrapper::populateButtons() {
    using std::begin;
    using std::end;
    using ButtonData                     = std::pair<std::string, NavButton>;
    const ButtonData button_name_pairs[] = {
        {"CTRL", NavButton::CTRL},
        {"FIT", NavButton::FIT},
        {"SHIFT", NavButton::SHIFT},
    };

    const auto b           = begin(button_name_pairs);
    const auto e           = end(button_name_pairs);
    const auto num_buttons = spndev_num_buttons(dev);
    buttons.resize(num_buttons);
    for(int button_idx = 0; button_idx < num_buttons; ++button_idx) {
        auto button_name = spndev_button_name(dev, button_idx);
        auto it =
            std::find_if(b, e, [&](ButtonData const& data) { return data.first == button_name; });
        if(it != e) {
            buttons[button_idx] = it->second;
        }
    }
}

#else

NavDeviceWrapper::NavDeviceWrapper(const char* device) {
}
NavDeviceWrapper::~NavDeviceWrapper() {
}

bool NavDeviceWrapper::process(SolveSpace::Platform::SixDofEvent&) {
    return false;
}
#endif