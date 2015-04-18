#ifndef RVEMANAGER
#define RVEMANAGER

#include <map>

#include "UI/userinterfacemanager.h"    // Include it first to fix OpenGL/GLEW compatibility
#include "console.h"
#include "representativevolumeelement.h"
#include "consolecommand.h"

/// \todo a lot of refactoring
namespace Controller
{

class RepresentativeVolumeElementConsoleInterface;

/// editRVE --------------------------------------------------------------------------------
/// \warning don't make direct usage! it should be nested, but in that case causes Qt error
class _EditRVECommand : public QObject, public ConsoleCommand
{
    Q_OBJECT

    private: RepresentativeVolumeElementConsoleInterface &_manager;
    private: std::string _RVEName;
    public: _EditRVECommand(
            RepresentativeVolumeElementConsoleInterface &manager,
            Console &console) :
        ConsoleCommand(
            "editRVE",
            "editRVE <Name>\n"
            "Call the Graphical User Interface Representative Volume Element edit form.\n"
            "Arguments:\n"
            " <Name> - the name of RVE in RAM memory.\n",
            console),
        _manager(manager)
    {
        connect(this, SIGNAL(signal_editRVEGUIStart(RepresentativeVolumeElement*)),
                &UserInterface::UserInterfaceManager::instance(),
                SLOT(editRVEGUIStart(RepresentativeVolumeElement*)));
        connect(&UserInterface::UserInterfaceManager::instance(),
                SIGNAL(signal_editRVEGUIError()),
                this, SLOT(editRVEGUIError()));
        connect(&UserInterface::UserInterfaceManager::instance(),
                SIGNAL(signal_editRVEGUIFinish()),
                this, SLOT(editRVEGUIFinish()));

        connect(&UserInterface::UserInterfaceManager::instance(),
                SIGNAL(signal_applyGaussFltrRVE_T(int,float,float,float)),
                this, SLOT(applyGaussFltrRVE(int,float,float,float)));

        connect(this, SIGNAL(signal_applyGaussFltrRVEDone()),
                &UserInterface::UserInterfaceManager::instance(),
                SIGNAL(signal_applyGaussFltrRVEDone_T()));
    }
    public: Q_SIGNAL void signal_editRVEGUIStart(RepresentativeVolumeElement* ptrToRVE);
    public: Q_SLOT void editRVEGUIFinish(){
        getConsole().writeToOutput("Edit RVE GUI finish.\n");}
    public: Q_SLOT void editRVEGUIError(){
        getConsole().writeToOutput("Edit RVE GUI is already running.\n");}

    public: Q_SLOT void applyGaussFltrRVE(
            int discreteRadius,
            float ellipsoidScaleFactorX,
            float ellipsoidScaleFactorY,
            float ellipsoidScaleFactorZ);
    public: Q_SIGNAL void signal_applyGaussFltrRVEDone();

    public: int executeConsoleCommand(const std::vector<std::string> &argv) override;
};

class RepresentativeVolumeElementConsoleInterface : public QObject
{
    Q_OBJECT

    public : std::map<std::string, RepresentativeVolumeElement*> RVEs;

    /// createRVE ----------------------------------------------------------------------------
    public : std::string createRVE(const std::string &name, int size) noexcept;
    private: class _CreateRVECommand : public ConsoleCommand
    {
        private: RepresentativeVolumeElementConsoleInterface &_manager;
        public: _CreateRVECommand(
                RepresentativeVolumeElementConsoleInterface &manager, Console &console) :
            ConsoleCommand(
                "createRVE",
                "createRVE <Name> <size>\n"
                "Creates Representative Volume Element (RVE) object in RAM memory.\n"
                "Arguments:\n"
                " <Name> - the name of RVE in RAM memory,\n"
                " <size> - the discrete size of RVE. It should be equal to some power of two.\n",
                console),
                _manager(manager){}
        public: int executeConsoleCommand(const std::vector<std::string> &argv) override;
    } *_commandCreateRVE = nullptr;

    /// deleteRVE ----------------------------------------------------------------------------
    public : std::string deleteRVE(const std::string &name) noexcept;
    private: class _DeleteRVECommand : public ConsoleCommand
    {
        private: RepresentativeVolumeElementConsoleInterface &_manager;
        public: _DeleteRVECommand(
                RepresentativeVolumeElementConsoleInterface &manager, Console &console) :
            ConsoleCommand(
                "deleteRVE",
                "deleteRVE <Name>\n"
                "Deletes Representative Volume Element (RVE) object from RAM memory.\n"
                "Arguments:\n"
                " <Name> - the name of RVE in RAM memory.\n",
                console),
                _manager(manager){}
        public: int executeConsoleCommand(const std::vector<std::string> &argv) override;
    } *_commandDeleteRVE = nullptr;

    /// printRVE -----------------------------------------------------------------------------
    public : std::string printRVE() noexcept;
    private: class _PrintRVECommand : public ConsoleCommand
    {
        private: RepresentativeVolumeElementConsoleInterface &_manager;
        public: _PrintRVECommand(
                RepresentativeVolumeElementConsoleInterface &manager, Console &console) :
            ConsoleCommand(
                "printRVE",
                "createRVE\n"
                "Prints existing in RAM memory Representative Volume Element (RVE) objects in\n"
                "format: [<index>]: <name> <size>.\n"
                "Takes no arguments.\n",
                console),
                _manager(manager){}
        public: int executeConsoleCommand(const std::vector<std::string> &argv) override;
    } *_commandPrintRVE = nullptr;

    _EditRVECommand *_commandEditRVE = nullptr;

    /// cleanRVE -------------------------------------------------------------------------
    public : std::string cleanRVE(const std::string &name) noexcept;
    private: class _cleanRVECommand : public ConsoleCommand
    {
        private: RepresentativeVolumeElementConsoleInterface &_manager;
        public: _cleanRVECommand(
                RepresentativeVolumeElementConsoleInterface &manager, Console &console) :
            ConsoleCommand(
                "cleanRVE",
                "cleanRVE <Name>\n"
                "Clean RVE by set all it elements to 0.\n"
                "Arguments:\n"
                " <Name> - the name of RVE in RAM memory.\n",
                console),
                _manager(manager){}
        public: int executeConsoleCommand(const std::vector<std::string> &argv) override;
    } *_commandCleanRVE = nullptr;

    /// cleanUnMaskedRVE -------------------------------------------------------------------------
    public : std::string cleanUnMaskedRVE(const std::string &name, float filler) noexcept;
    private: class _cleanUnMaskedRVECommand : public ConsoleCommand
    {
        private: RepresentativeVolumeElementConsoleInterface &_manager;
        public: _cleanUnMaskedRVECommand(
                RepresentativeVolumeElementConsoleInterface &manager, Console &console) :
            ConsoleCommand(
                "cleanUnMaskedRVE",
                "cleanUnMaskedRVE <Name> <filler>\n"
                "Clean un masked elements RVE by set them to <filler>.\n"
                "Arguments:\n"
                " <Name> - the name of RVE in RAM memory;\n"
                " <filler> - (optional) value to fill RVE elements (default = 0).\n",
                console),
                _manager(manager){}
        public: int executeConsoleCommand(const std::vector<std::string> &argv) override;
    } *_commandUnMaskedCleanRVE = nullptr;

    /// cleanMaskRVE -------------------------------------------------------------------------
    public : std::string cleanMaskRVE(const std::string &name) noexcept;
    private: class _cleanMaskRVECommand : public ConsoleCommand
    {
        private: RepresentativeVolumeElementConsoleInterface &_manager;
        public: _cleanMaskRVECommand(
                RepresentativeVolumeElementConsoleInterface &manager, Console &console) :
            ConsoleCommand(
                "cleanMaskRVE",
                "cleanMaskRVE <Name>\n"
                "Clean RVE mask.\n"
                "Arguments:\n"
                " <Name> - the name of RVE in RAM memory.\n",
                console),
                _manager(manager){}
        public: int executeConsoleCommand(const std::vector<std::string> &argv) override;
    } *_commandCleanMaskRVE = nullptr;

    /// addRandomNoiseRVE -------------------------------------------------------------------------
    public : std::string addRandomNoiseRVE(const std::string &name) noexcept;
    private: class _addRandomNoiseRVECommandmmand : public ConsoleCommand
    {
        private: RepresentativeVolumeElementConsoleInterface &_manager;
        public: _addRandomNoiseRVECommandmmand(
                RepresentativeVolumeElementConsoleInterface &manager, Console &console) :
            ConsoleCommand(
                "addRandomNoiseRVE",
                "addRandomNoiseRVE <Name>\n"
                "Add random noise to RVE elements that are not masked by RVE mask.\n"
                "Note that RVE can be unnormalized after this call, "
                "if it wasn't cleaned before.\n"
                "Arguments:\n"
                " <Name> - the name of RVE in RAM memory.\n",
                console),
                _manager(manager){}
        public: int executeConsoleCommand(const std::vector<std::string> &argv) override;
    } *_commandAddRandomNoiseRVE = nullptr;

    /// normalizeUnMaskedRVE -----------------------------------------------------------------
    public : std::string normalizeUnMaskedRVE(const std::string &name) noexcept;
    private: class _normalizeUnMaskedRVECommand : public ConsoleCommand
    {
        private: RepresentativeVolumeElementConsoleInterface &_manager;
        public: _normalizeUnMaskedRVECommand(
                RepresentativeVolumeElementConsoleInterface &manager, Console &console) :
            ConsoleCommand(
                "normalizeUnMaskedRVE",
                "normalizeUnMaskedRVE <Name>\n"
                "Normalize un msked RVE elements.\n"
                "Arguments:\n"
                " <Name> - the name of RVE in RAM memory.\n",
                console),
                _manager(manager){}
        public: int executeConsoleCommand(const std::vector<std::string> &argv) override;
    } *_commandNormalizeUnMaskedRVE = nullptr;

    /// invertUnMaskedRVE ---------------------------------------------------------------------
    public : std::string invertUnMaskedRVE(const std::string &name) noexcept;
    private: class _invertUnMaskedRVECommand : public ConsoleCommand
    {
        private: RepresentativeVolumeElementConsoleInterface &_manager;
        public: _invertUnMaskedRVECommand(
                RepresentativeVolumeElementConsoleInterface &manager, Console &console) :
            ConsoleCommand(
                "invertUnMaskedRVE",
                "invertUnMaskedRVE <Name>\n"
                "Invert normalized un masked RVE elements.\n"
                "Arguments:\n"
                " <Name> - the name of RVE in RAM memory.\n",
                console),
                _manager(manager){}
        public: int executeConsoleCommand(const std::vector<std::string> &argv) override;
    } *_commandInvertUnMaskedRVE = nullptr;

    /// applyGaussianFilterRVE -------------------------------------------------------------------------
    public : std::string applyGaussianFilterRVE(
            const std::string &name,
            int discreteRadius,
            float ellipsoidScaleFactorX,
            float ellipsoidScaleFactorY,
            float ellipsoidScaleFactorZ,
            bool useDataAsIntensity,
            float intensityFactor);
    private: class _applyGaussianFilterRVECommand : public ConsoleCommand
    {
        private: RepresentativeVolumeElementConsoleInterface &_manager;
        public: _applyGaussianFilterRVECommand(
                RepresentativeVolumeElementConsoleInterface &manager, Console &console) :
            ConsoleCommand(
                "applyGaussianFilterRVE",
                "applyGaussianFilterRVE <Name> <Radius> <ScaleFactorX> "
                "<ScaleFactorY> <ScaleFactorZ> <useDataAsIntensity> <intensityFactor>\n"
                "Apply Gaussian blur filter to given Representative "
                "Volume Element elements.\n"
                "Arguments:\n"
                " <Name> - the name of RVE in RAM memory;\n"
                " <Radius> - the radius of filter mask, should be > 0;\n"
                " <ScaleFactorX> - (optional) the ellipsoid scale factor on X axis of "
                "filter mask, should be > 0 and <= 1;\n"
                " <ScaleFactorY> - (optional) the ellipsoid scale factor on Y axis of "
                "filter mask, should be > 0 and <= 1;\n"
                " <ScaleFactorZ> - (optional) the ellipsoid scale factor on Z axis of "
                "filter mask, should be > 0 and <= 1;\n"
                " <useDataAsIntensity> - (optional) use RVE elements as intensity "
                "'true' or 'false';\n"
                " <intensityFactor> - (optional) if <useDataAsIntensity> equal 'true',"
                "use intensity multiply factor.\n",
                console),
                _manager(manager){}
        public: int executeConsoleCommand(const std::vector<std::string> &argv) override;
    } *_commandApplyGaussianFilterRVE = nullptr;

    /// applyTwoCutMaskInsideRVE ----------------------------------------------------------
    public : std::string applyTwoCutMaskInsideRVE(
            const std::string &name,
            float cutLevelA,
            float cutLevelB) noexcept;
    private: class _applyTwoCutMaskInsideRVECommand : public ConsoleCommand
    {
        private: RepresentativeVolumeElementConsoleInterface &_manager;
        public: _applyTwoCutMaskInsideRVECommand(
                RepresentativeVolumeElementConsoleInterface &manager, Console &console) :
            ConsoleCommand(
                "applyTwoCutMaskInsideRVE",
                "applyTwoCutMaskInsideRVE <Name> <cutLevelA> <cutLevelB>\n"
                "Apply two-cut to mask (inside).\n"
                "Arguments:\n"
                " <Name> - the name of RVE in RAM memory;\n"
                " <cutLevelA> - bottom cut level;"
                " <cutLevelB> - top cut level;"
                " 0.0f <= cutlevelA < cutLevelB <= 1.0f.",
                console),
                _manager(manager){}
        public: int executeConsoleCommand(const std::vector<std::string> &argv) override;
    } *_commandApplyTwoCutMaskInsideRVE = nullptr;

    /// applyTwoCutMaskOutsideRVE ----------------------------------------------------------
    public : std::string applyTwoCutMaskOutsideRVE(
            const std::string &name,
            float cutLevelA,
            float cutLevelB) noexcept;
    private: class _applyTwoCutMaskOutsideRVECommand : public ConsoleCommand
    {
        private: RepresentativeVolumeElementConsoleInterface &_manager;
        public: _applyTwoCutMaskOutsideRVECommand(
                RepresentativeVolumeElementConsoleInterface &manager, Console &console) :
            ConsoleCommand(
                "applyTwoCutMaskOutsideRVE",
                "applyTwoCutMaskOutsideRVE <Name> <cutLevelA> <cutLevelB>\n"
                "Apply two-cut to mask (outside).\n"
                "Arguments:\n"
                " <Name> - the name of RVE in RAM memory;\n"
                " <cutLevelA> - bottom cut level;"
                " <cutLevelB> - top cut level;"
                " 0.0f <= cutlevelA < cutLevelB <= 1.0f.",
                console),
                _manager(manager){}
        public: int executeConsoleCommand(const std::vector<std::string> &argv) override;
    } *_commandApplyTwoCutMaskOutsideRVE = nullptr;

    /// Constructor --------------------------------------------------------------------------
    public : RepresentativeVolumeElementConsoleInterface(Console &console):
        _commandCreateRVE(new _CreateRVECommand(*this, console)),
        _commandDeleteRVE(new _DeleteRVECommand(*this, console)),
        _commandPrintRVE(new _PrintRVECommand(*this, console)),
        _commandEditRVE(new _EditRVECommand(*this, console)),
        _commandCleanRVE(new _cleanRVECommand(*this, console)),
        _commandUnMaskedCleanRVE(new _cleanUnMaskedRVECommand(*this, console)),
        _commandCleanMaskRVE(new _cleanMaskRVECommand(*this, console)),
        _commandAddRandomNoiseRVE(new _addRandomNoiseRVECommandmmand(*this, console)),
        _commandNormalizeUnMaskedRVE(new _normalizeUnMaskedRVECommand(*this, console)),
        _commandInvertUnMaskedRVE(new _invertUnMaskedRVECommand(*this, console)),
        _commandApplyGaussianFilterRVE(new _applyGaussianFilterRVECommand(*this, console)),
        _commandApplyTwoCutMaskInsideRVE(new _applyTwoCutMaskInsideRVECommand(*this, console)),
        _commandApplyTwoCutMaskOutsideRVE(new _applyTwoCutMaskOutsideRVECommand(*this, console))
        {}

    /// Destructor ---------------------------------------------------------------------------
    public : ~RepresentativeVolumeElementConsoleInterface()
    {
        for(auto _rve: RVEs)
            delete _rve.second;
        delete _commandCreateRVE;
        delete _commandDeleteRVE;
        delete _commandPrintRVE;
        delete _commandEditRVE;
        delete _commandCleanRVE;
        delete _commandUnMaskedCleanRVE;
        delete _commandCleanMaskRVE;
        delete _commandAddRandomNoiseRVE;
        delete _commandNormalizeUnMaskedRVE;
        delete _commandInvertUnMaskedRVE;
        delete _commandApplyGaussianFilterRVE;
        delete _commandApplyTwoCutMaskInsideRVE;
        delete _commandApplyTwoCutMaskOutsideRVE;
    }
};
}

#endif // RVEMANAGER
