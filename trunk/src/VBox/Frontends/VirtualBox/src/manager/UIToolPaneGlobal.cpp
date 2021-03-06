/* $Id$ */
/** @file
 * VBox Qt GUI - UIToolPaneGlobal class implementation.
 */

/*
 * Copyright (C) 2017 Oracle Corporation
 *
 * This file is part of VirtualBox Open Source Edition (OSE), as
 * available from http://www.virtualbox.org. This file is free software;
 * you can redistribute it and/or modify it under the terms of the GNU
 * General Public License (GPL) as published by the Free Software
 * Foundation, in version 2 as it comes in the "COPYING" file of the
 * VirtualBox OSE distribution. VirtualBox OSE is distributed in the
 * hope that it will be useful, but WITHOUT ANY WARRANTY of any kind.
 */

#ifdef VBOX_WITH_PRECOMPILED_HEADERS
# include <precomp.h>
#else  /* !VBOX_WITH_PRECOMPILED_HEADERS */

/* Qt includes: */
# include <QStackedLayout>
# ifndef VBOX_WS_MAC
#  include <QStyle>
# endif
# include <QUuid>

/* GUI includes */
# include "UIActionPoolSelector.h"
# include "UIWelcomePane.h"
# include "UIHostNetworkManager.h"
# include "UIIconPool.h"
# include "UIMediumManager.h"
# include "UIToolPaneGlobal.h"

/* Other VBox includes: */
# include <iprt/assert.h>

#endif /* !VBOX_WITH_PRECOMPILED_HEADERS */


UIToolPaneGlobal::UIToolPaneGlobal(UIActionPool *pActionPool, QWidget *pParent /* = 0 */)
    : QIWithRetranslateUI<QWidget>(pParent)
    , m_pActionPool(pActionPool)
    , m_pLayout(0)
    , m_pPaneDesktop(0)
    , m_pPaneMedia(0)
    , m_pPaneNetwork(0)
{
    /* Prepare: */
    prepare();
}

UIToolPaneGlobal::~UIToolPaneGlobal()
{
    /* Cleanup: */
    cleanup();
}

ToolTypeGlobal UIToolPaneGlobal::currentTool() const
{
    return m_pLayout->currentWidget()->property("ToolType").value<ToolTypeGlobal>();
}

bool UIToolPaneGlobal::isToolOpened(ToolTypeGlobal enmType) const
{
    /* Search through the stacked widgets: */
    for (int iIndex = 0; iIndex < m_pLayout->count(); ++iIndex)
        if (m_pLayout->widget(iIndex)->property("ToolType").value<ToolTypeGlobal>() == enmType)
            return true;
    return false;
}

void UIToolPaneGlobal::openTool(ToolTypeGlobal enmType)
{
    /* Search through the stacked widgets: */
    int iActualIndex = -1;
    for (int iIndex = 0; iIndex < m_pLayout->count(); ++iIndex)
        if (m_pLayout->widget(iIndex)->property("ToolType").value<ToolTypeGlobal>() == enmType)
            iActualIndex = iIndex;

    /* If widget with such type exists: */
    if (iActualIndex != -1)
    {
        /* Activate corresponding index: */
        m_pLayout->setCurrentIndex(iActualIndex);
    }
    /* Otherwise: */
    else
    {
        /* Create, remember, append corresponding stacked widget: */
        switch (enmType)
        {
            case ToolTypeGlobal_Desktop:
            {
                /* Create Desktop pane: */
                m_pPaneDesktop = new UIWelcomePane;
                if (m_pPaneDesktop)
                {
                    /* Configure pane: */
                    m_pPaneDesktop->setProperty("ToolType", QVariant::fromValue(ToolTypeGlobal_Desktop));

                    /* Add into layout: */
                    m_pLayout->addWidget(m_pPaneDesktop);
                    m_pLayout->setCurrentWidget(m_pPaneDesktop);

                    /* Retranslate Desktop pane: */
                    retranslateDesktopPane();
                }
                break;
            }
            case ToolTypeGlobal_VirtualMedia:
            {
                /* Create Virtual Media Manager: */
                m_pPaneMedia = new UIMediumManagerWidget(EmbedTo_Stack, m_pActionPool, false /* show toolbar */);
                AssertPtrReturnVoid(m_pPaneMedia);
                {
#ifndef VBOX_WS_MAC
                    const int iMargin = qApp->style()->pixelMetric(QStyle::PM_LayoutLeftMargin) / 4;
                    m_pPaneMedia->setContentsMargins(iMargin, 0, iMargin, 0);
#endif

                    /* Configure pane: */
                    m_pPaneMedia->setProperty("ToolType", QVariant::fromValue(ToolTypeGlobal_VirtualMedia));

                    /* Add into layout: */
                    m_pLayout->addWidget(m_pPaneMedia);
                    m_pLayout->setCurrentWidget(m_pPaneMedia);
                }
                break;
            }
            case ToolTypeGlobal_HostNetwork:
            {
                /* Create Host Network Manager: */
                m_pPaneNetwork = new UIHostNetworkManagerWidget(EmbedTo_Stack, m_pActionPool, false /* show toolbar */);
                AssertPtrReturnVoid(m_pPaneNetwork);
                {
#ifndef VBOX_WS_MAC
                    const int iMargin = qApp->style()->pixelMetric(QStyle::PM_LayoutLeftMargin) / 4;
                    m_pPaneNetwork->setContentsMargins(iMargin, 0, iMargin, 0);
#endif

                    /* Configure pane: */
                    m_pPaneNetwork->setProperty("ToolType", QVariant::fromValue(ToolTypeGlobal_HostNetwork));

                    /* Add into layout: */
                    m_pLayout->addWidget(m_pPaneNetwork);
                    m_pLayout->setCurrentWidget(m_pPaneNetwork);
                }
                break;
            }
            default:
                AssertFailedReturnVoid();
        }
    }
}

void UIToolPaneGlobal::closeTool(ToolTypeGlobal enmType)
{
    /* Search through the stacked widgets: */
    int iActualIndex = -1;
    for (int iIndex = 0; iIndex < m_pLayout->count(); ++iIndex)
        if (m_pLayout->widget(iIndex)->property("ToolType").value<ToolTypeGlobal>() == enmType)
            iActualIndex = iIndex;

    /* If widget with such type doesn't exist: */
    if (iActualIndex != -1)
    {
        /* Forget corresponding widget: */
        switch (enmType)
        {
            case ToolTypeGlobal_Desktop:      m_pPaneDesktop = 0; break;
            case ToolTypeGlobal_VirtualMedia: m_pPaneMedia = 0; break;
            case ToolTypeGlobal_HostNetwork:  m_pPaneNetwork = 0; break;
            default: break;
        }
        /* Delete corresponding widget: */
        QWidget *pWidget = m_pLayout->widget(iActualIndex);
        m_pLayout->removeWidget(pWidget);
        delete pWidget;
    }
}

void UIToolPaneGlobal::setDetailsError(const QString &strError)
{
    /* Update desktop pane: */
    if (m_pPaneDesktop)
        m_pPaneDesktop->updateDetailsError(strError);
}

void UIToolPaneGlobal::retranslateUi()
{
    retranslateDesktopPane();
}

void UIToolPaneGlobal::prepare()
{
    /* Create stacked-layout: */
    m_pLayout = new QStackedLayout(this);

    /* Create desktop pane: */
    openTool(ToolTypeGlobal_VirtualMedia);

    /* Apply language settings: */
    retranslateUi();
}

void UIToolPaneGlobal::cleanup()
{
    /* Remove all widgets prematurelly: */
    while (m_pLayout->count())
    {
        QWidget *pWidget = m_pLayout->widget(0);
        m_pLayout->removeWidget(pWidget);
        delete pWidget;
    }
}

void UIToolPaneGlobal::retranslateDesktopPane()
{
    if (!m_pPaneDesktop)
        return;

    /* Translate Global Tools welcome screen: */
    m_pPaneDesktop->setToolsPaneIcon(UIIconPool::iconSet(":/tools_banner_global_200px.png"));
    m_pPaneDesktop->setToolsPaneText(
        tr("<h3>Welcome to VirtualBox!</h3>"
           "<p>This window represents a set of global tools "
           "which are currently opened (or can be opened). "
           "They are not related to any particular machine but "
           "to the complete VM collection. For a list of currently "
           "available tools check the corresponding menu at the right "
           "side of the main tool bar located at the top of the window. "
           "This list will be extended with new tools in future releases.</p>"
           "<p>You can press the <b>%1</b> key to get instant help, or visit "
           "<a href=https://www.virtualbox.org>www.virtualbox.org</a> "
           "for more information and latest news.</p>")
           .arg(QKeySequence(QKeySequence::HelpContents).toString(QKeySequence::NativeText)));

    /* Wipe out the tool descriptions: */
    m_pPaneDesktop->removeToolDescriptions();

    /* Add tool descriptions: */
    QAction *pAction1 = m_pActionPool->action(UIActionIndexST_M_Tools_M_Global_S_VirtualMediaManager);
    m_pPaneDesktop->addToolDescription(pAction1,
                                       tr("Tool to observe virtual storage media. "
                                          "Reflects all the chains of virtual disks you have registered "
                                          "(per each storage type) within your virtual machines and allows for media "
                                          "operations like copy, remove, release "
                                          "(detach it from VMs where it is currently attached to) and observe their properties. "
                                          "Allows to edit medium attributes like type, "
                                          "location/name, description and size (for dynamical storages "
                                          "only)."));
    QAction *pAction2 = m_pActionPool->action(UIActionIndexST_M_Tools_M_Global_S_HostNetworkManager);
    m_pPaneDesktop->addToolDescription(pAction2,
                                       tr("Tool to control host-only network interfaces. "
                                          "Reflects host-only networks, their DHCP servers and allows "
                                          "for operations on the networks like possibility to create, remove "
                                          "and observe their properties. Allows to edit various "
                                          "attributes for host-only interface and corresponding DHCP server."));
}
