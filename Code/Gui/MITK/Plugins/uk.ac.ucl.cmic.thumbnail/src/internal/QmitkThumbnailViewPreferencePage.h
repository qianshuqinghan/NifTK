/*=============================================================================

 NifTK: An image processing toolkit jointly developed by the
             Dementia Research Centre, and the Centre For Medical Image Computing
             at University College London.

 See:        http://dementia.ion.ucl.ac.uk/
             http://cmic.cs.ucl.ac.uk/
             http://www.ucl.ac.uk/

 Last Changed      : $Date$
 Revision          : $Revision$
 Last modified by  : $Author$

 Original author   : m.clarkson@ucl.ac.uk

 Copyright (c) UCL : See LICENSE.txt in the top level directory for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 ============================================================================*/

#ifndef _QMITKTHUMBNAILVIEWPREFERENCE_PAGE_H_INCLUDED
#define _QMITKTHUMBNAILVIEWPREFERENCE_PAGE_H_INCLUDED

#include "berryIQtPreferencePage.h"
#include "uk_ac_ucl_cmic_thumbnail_Export.h"
#include <berryIPreferences.h>

class QWidget;
class QSpinBox;
class QDoubleSpinBox;
class QCheckBox;
class QPushButton;

/**
 * \class QmitkThumbnailViewPreferencePage
 * \brief Preferences page for the ThumbnailView plugin, enabling the user to set colour,
 * line thickness, opacity, layer, and whether to respond to mouse events.
 * \ingroup uk_ac_ucl_cmic_gui_thumbnail
 *
 */
class THUMBNAIL_EXPORT QmitkThumbnailViewPreferencePage : public QObject, public berry::IQtPreferencePage
{
  Q_OBJECT
  Q_INTERFACES(berry::IPreferencePage)

public:

  QmitkThumbnailViewPreferencePage();
  QmitkThumbnailViewPreferencePage(const QmitkThumbnailViewPreferencePage& other);
  ~QmitkThumbnailViewPreferencePage();

  void CreateQtControl(QWidget* widget);
  QWidget* GetQtControl() const;

  /// \brief Nothing to do.
  void Init(berry::IWorkbench::Pointer workbench);

  /// \see IPreferencePage::PerformOk()
  virtual bool PerformOk();

  /// \see IPreferencePage::PerformCancel()
  virtual void PerformCancel();

  /// \see IPreferencePage::Update()
  virtual void Update();

  /// \brief Stores the name of the preferences node that stores the box outline colour
  static const std::string THUMBNAIL_BOX_COLOUR;

  /// \brief Stores the name of the preferences name that stores the box outline colour style sheet.
  static const std::string THUMBNAIL_BOX_COLOUR_STYLE_SHEET;

  /// \brief Stores the name of the preferences name that stores the box line thickness
  static const std::string THUMBNAIL_BOX_THICKNESS;

  /// \brief Stores the name of the preferences name that stores the box opacity
  static const std::string THUMBNAIL_BOX_OPACITY;

  /// \brief Stores the name of the preferences name that stores the box layer.
  static const std::string THUMBNAIL_BOX_LAYER;

protected slots:

  void OnBoxColourChanged();
  void OnResetBoxColour();

protected:

  QWidget        *m_MainControl;
  QSpinBox       *m_BoxThickness;
  QDoubleSpinBox *m_BoxOpacity;
  QSpinBox       *m_BoxLayer;
  QPushButton    *m_BoxColorPushButton;
  QString         m_BoxColorStyleSheet;
  std::string     m_BoxColor;

  bool m_Initializing;

  berry::IPreferences::Pointer m_ThumbnailPreferencesNode;
};

#endif /* _QMITKTHUMBNAILVIEWPREFERENCE_PAGE_H_INCLUDED */

