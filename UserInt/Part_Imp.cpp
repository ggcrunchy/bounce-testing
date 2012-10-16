#include "UserInterface.h"

namespace UI
{
	/// @brief Constructs a Part object
	/// @param owner Widget used to build part
	Part::Part (Widget * owner) : Signalee(), mOwner(owner)
	{
	}

	/// @brief Destructs a Part object
	Part::~Part (void)
	{
		mOwner->mParts.erase(this);
	}

	/// @brief Gets the part's owner
	/// @return Owner widget
	Widget * Part::GetOwner (void)
	{
		return mOwner;
	}
}