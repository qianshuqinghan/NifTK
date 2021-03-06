/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#ifndef niftkBaseApplication_h
#define niftkBaseApplication_h

#include "niftkCoreAppExports.h"

#include <mitkBaseApplication.h>


namespace Poco
{
namespace Util
{
class HelpFormatter;
}
}


namespace niftk
{

/// \brief Base class for BlueBerry based GUI applications for NifTK.
///
/// This class defines some generic command line options that can be
/// useful for any application. The values provided for these options
/// are put in the application context, and can be accessed from the
/// plugin context of any plugin. The name of the context properties
/// are defined by the static constant members of this class.
///
/// This class is not abstract. You can instantiate this class directly.
/// You only need to set the application name and the PROP_APPLICATION
/// property on an application instance to run it.
///
/// However, if you want to define application specific command line
/// options, you need to derive a new class from this and override the
/// 'defineOptions' function.
///
/// The here defined options are processed in the uk.ac.ucl.cmic.commonapps
/// plugin.
///
/// Supported options:
///
///   --open, -o <name>:<file>
///
///         opens the file with the given name
///
///   --derives-from, -d <source data name>:<data name>[,<data name>]...
///
///         makes the data nodes derive from (the 'children' of) the source data
///
///   --property, -p <data name>:<property>=<value>[,<property>=<value>]...
///
///         sets properties of a data node to the give values
///
///   --perspective, -P <perspective>
///
///         sets the initial perspective
///
class NIFTKCOREAPP_EXPORT BaseApplication : public mitk::BaseApplication
{
public:

  static const QString PROP_OPEN;
  static const QString PROP_DERIVES_FROM;
  static const QString PROP_PROPERTY;
  static const QString PROP_PERSPECTIVE;
  static const QString PROP_RESET_PERSPECTIVE;
  static const QString PROP_VERSION;
  static const QString PROP_PRODUCT_NAME;

  BaseApplication(int argc, char **argv);

  /// Runs the application.
  /// Like mitk::BaseApplication::run(), but it exits clean after Poco exceptions.
  int run() override;

protected:

  /// \brief Define command line arguments
  /// \param options
  void defineOptions(Poco::Util::OptionSet& options) override;

  /// \brief Prints the version number and exits.
  void PrintVersion(const std::string& name, const std::string& value);

  /// \brief Collects values for repeatable options and stores them as a list
  ///
  /// It works similarly to the binding function. However, the binding function
  /// does not care if the option is repeatable or not. In case of repetition,
  /// the last supplied value is kept as a property of the framework context.
  ///
  /// This function collects the values for a property and stores them as a
  /// QStringList.
  ///
  /// \param name The name of the option. It is the long form even if the short
  ///             form is applied.
  /// \param value The value specified for the option.
  void HandleRepeatableOption(const std::string& name, const std::string& value);

  /// \brief Overwrites the description of existing options for prettier formatting.
  /// Capitalises the initial letter, adds a period to the end, and puts an extra
  /// new line before and after the description.
  void ReformatOptionDescriptions(Poco::Util::OptionSet& options);

  virtual Poco::Util::HelpFormatter* CreateHelpFormatter();

private:

  /// \brief Callback function for the '--help' option.
  /// The callback function of the base class is replaced to this implementation,
  /// so that we can define a header, footer, and can have longer description of
  /// options.
  void PrintHelp(const std::string& name, const std::string& value);

};

}

#endif
