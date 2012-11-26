/*=============================================================================

 NifTK: An image processing toolkit jointly developed by the
             Dementia Research Centre, and the Centre For Medical Image Computing
             at University College London.

 See:        http://dementia.ion.ucl.ac.uk/
             http://cmic.cs.ucl.ac.uk/
             http://www.ucl.ac.uk/

 Last Changed      : $Date: 2012-07-25 07:31:59 +0100 (Wed, 25 Jul 2012) $
 Revision          : $Revision: 9401 $
 Last modified by  : $Author: mjc $

 Original author   : m.clarkson@ucl.ac.uk

 Copyright (c) UCL : See LICENSE.txt in the top level directory for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 ============================================================================*/

#ifndef QMITKIGINETWORKDATASOURCE_H
#define QMITKIGINETWORKDATASOURCE_H

#include "niftkIGIGuiExports.h"
#include "QmitkIGIDataSource.h"
#include <OIGTLSocketObject.h>
#include <Common/NiftyLinkXMLBuilder.h>

/**
 * \class QmitkIGINetworkDataSource
 * \brief Base class for IGI Data Sources that are receiving networked input,
 * such as tracker tools, or even US/Video grabbers on remote machines.
 */
class NIFTKIGIGUI_EXPORT QmitkIGINetworkDataSource : public QmitkIGIDataSource
{
  Q_OBJECT

public:

  mitkClassMacro(QmitkIGINetworkDataSource, QmitkIGIDataSource);

  /**
   * \brief Sets the socket pointer.
   */
  itkSetObjectMacro(Socket, OIGTLSocketObject);

  /**
   * \brief Gets the socket pointer.
   */
  itkGetConstMacro(Socket, OIGTLSocketObject*);

  /**
   * \brief Sets the Client Descriptor XML.
   */
  itkSetObjectMacro(ClientDescriptor, ClientDescriptorXMLBuilder);

  /**
   * \brief Gets the Client Descriptor XML.
   */
  itkGetConstMacro(ClientDescriptor, ClientDescriptorXMLBuilder*);

  /**
   * \brief Returns the port number that this tool is using or -1 if no socket is available.
   */
  int GetPort() const;

  /**
   * \brief If there is a socket associated with this tool, will send the message.
   */
  void SendMessage(OIGTLMessage::Pointer msg);

public slots:

signals:

protected:

  QmitkIGINetworkDataSource(); // Purposefully hidden.
  virtual ~QmitkIGINetworkDataSource(); // Purposefully hidden.

  QmitkIGINetworkDataSource(const QmitkIGINetworkDataSource&); // Purposefully not implemented.
  QmitkIGINetworkDataSource& operator=(const QmitkIGINetworkDataSource&); // Purposefully not implemented.

private:

  OIGTLSocketObject           *m_Socket;
  ClientDescriptorXMLBuilder  *m_ClientDescriptor;

}; // end class

#endif

