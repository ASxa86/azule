#include <editor/DockWidgetEntity.h>

#include <age/core/Engine.h>
#include <age/core/Factory.h>
#include <age/core/PimplImpl.h>
#include <age/entity/Component.h>
#include <age/entity/Entity.h>
#include <age/entity/EntityDatabase.h>
#include <editor/Application.h>
#include <editor/DialogComponents.h>
#include <editor/TreeWidgetEntity.h>
#include <QtWidgets/QBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QMenu>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QToolButton>

#include <iostream>

using namespace age;
using namespace age::entity;

Q_DECLARE_METATYPE(age::entity::Entity*)
Q_DECLARE_METATYPE(age::entity::Component*)

struct DockWidgetEntity::Impl
{
	QMenu* contextMenu{nullptr};
};

DockWidgetEntity::DockWidgetEntity(QWidget* parent) : QDockWidget(parent)
{
	qRegisterMetaType<age::entity::Entity*>();
	qRegisterMetaType<age::entity::Component*>();

	const auto widget = new QWidget();
	const auto vLayout = new QVBoxLayout(widget);
	vLayout->setMargin(0);
	vLayout->setSpacing(0);

	const auto tbEntity = new QToolBar();
	tbEntity->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonIconOnly);

	const auto twEntity = new TreeWidgetEntity();
	twEntity->setHeaderHidden(true);
	twEntity->setSelectionMode(QAbstractItemView::SelectionMode::ExtendedSelection);
	twEntity->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
	twEntity->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);

	vLayout->addWidget(tbEntity);
	vLayout->addWidget(twEntity);
	this->setWidget(widget);

	const auto addEntity = [this, twEntity] {
		const auto manager = Application::Instance()->getEngine().getChild<EntityDatabase>();

		if(manager != nullptr)
		{
			manager->addEntity();
		}
	};

	this->connect(tbEntity->addAction(QIcon(":icons/plus.png"), "Add Entity"), &QAction::triggered, this, addEntity);

	this->pimpl->contextMenu = new QMenu(this);
	this->connect(this->pimpl->contextMenu->addAction("Add Node"), &QAction::triggered, this, addEntity);
	this->connect(this->pimpl->contextMenu->addAction("Delete Node(s)"), &QAction::triggered, this, [this, twEntity] {
		const auto items = twEntity->selectedItems();

		for(auto item : items)
		{
			switch(item->type())
			{
				case TreeWidgetEntity::ItemType::Entity:
				{
					auto entity = item->data(0, Qt::UserRole).value<age::entity::Entity*>();
					entity->remove();
				}
				break;

				case TreeWidgetEntity::ItemType::Component:
				{
					auto component = item->data(0, Qt::UserRole).value<age::entity::Component*>();
					component->remove();
				}
				break;
			}
		}
	});

	this->pimpl->contextMenu->addSeparator();
	auto actComponent = this->pimpl->contextMenu->addAction("Add Component");
	this->connect(actComponent, &QAction::triggered, this, [this, twEntity] {
		const auto items = twEntity->selectedItems();

		if(items.empty() == false)
		{
			const auto& item = items[0];
			const auto entity = item->data(0, Qt::UserRole).value<age::entity::Entity*>();
			const auto dlgComponents = new DialogComponents(entity);
			dlgComponents->setModal(true);
			dlgComponents->setAttribute(Qt::WA_DeleteOnClose);
			dlgComponents->resize(500, 500);
			dlgComponents->show();
		}
	});

	this->connect(twEntity, &TreeWidgetEntity::customContextMenuRequested, this, [this, twEntity, actComponent](const QPoint& pos) {
		actComponent->setEnabled(false);

		const auto items = twEntity->selectedItems();

		if(items.size() == 1)
		{
			const auto& item = items[0];

			if(item != nullptr && item->type() == TreeWidgetEntity::ItemType::Entity)
			{
				actComponent->setEnabled(true);
			}
		}

		this->pimpl->contextMenu->popup(twEntity->mapToGlobal(pos));
	});
}

DockWidgetEntity::~DockWidgetEntity()
{
}