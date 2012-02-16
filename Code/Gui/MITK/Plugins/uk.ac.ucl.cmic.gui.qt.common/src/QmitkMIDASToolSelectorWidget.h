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

#ifndef QMITKMIDASTOOLSELECTORWIDGET_H
#define QMITKMIDASTOOLSELECTORWIDGET_H

#include <QWidget>
#include "ui_QmitkMIDASToolSelector.h"

/**
 * \class QmitkMIDASToolSelectorWidget
 * \brief Implements a widget containing the QmitkToolGUIArea and QmitkToolSelectionBox.
 *
 * This widget provides SetEnabled() and GetEnabled(), which simplify the idea
 * of enabling/disabling the toolbox.
 *
 * \ingroup uk_ac_ucl_cmic_gui_qt_common
 */
class QmitkMIDASToolSelectorWidget : public QWidget, public Ui::QmitkMIDASToolSelector {

  Q_OBJECT

public:

  /// \brief Constructor.
  QmitkMIDASToolSelectorWidget(QWidget *parent = 0);

  /// \brief Destructor.
  ~QmitkMIDASToolSelectorWidget();

  /// \brief Set the widget to be enabled / disabled.
  void SetEnabled(bool enabled);

  /// \brief Get the enabled status.
  bool GetEnabled() const;

  /// \brief Retrieves the current tool ID.
  int GetActiveToolID();

signals:

  /// \brief Emits the tool selected signal from the contained QmitkToolSelectionBox.
  void ToolSelected(int);

public slots:

  /// \brief We connect the QmitkToolSelectionBox ToolSelected signal to this OnToolSelected slot.
  void OnToolSelected(int);

protected:

private:

  QmitkMIDASToolSelectorWidget(const QmitkMIDASToolSelectorWidget&);  // Purposefully not implemented.
  void operator=(const QmitkMIDASToolSelectorWidget&);  // Purposefully not implemented.

};

#endif

