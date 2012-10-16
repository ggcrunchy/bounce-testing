#include "UserInterface.h"

namespace UI
{
	/// @brief Adds the widget to its frame
	/// @note NOP if invoked from an update or signal callback
	void Widget::AddToFrame (void)
	{
		if (eSignalTesting == mState->mMode) return;
		if (eUpdating == mState->mMode) return;

		// Unload the widget if it is already loaded.
		Unload();

		// Put the widget in the frame. Indicate that it is framed.
		mLookup = mState->mFrame.insert(mState->mFrame.end(), this);

		mStatus.set(eFramed);
	}

	/// @brief Allows or disallows signaling docked widgets
	/// @param bAllow If true, allow signaling
	void Widget::AllowDockSignal (bool bAllow)
	{
		mStatus.set(eCannotDockSignal, !bAllow);
	}

	/// @brief Allows or disallows updating loaded widgets
	/// @param bAllow If true, allow updating
	void Widget::AllowDockUpdate (bool bAllow)
	{
		mStatus.set(eCannotDockUpdate, !bAllow);
	}

	/// @brief Allows or disallows signaling this widget
	/// @param bAllow If true, allow signaling
	void Widget::AllowSignal (bool bAllow)
	{
		mStatus.set(eCannotSignal, !bAllow);
	}

	/// @brief Allows or disallows updating this widget
	/// @param bAllow If true, allow updating
	void Widget::AllowUpdate (bool bAllow)
	{
		mStatus.set(eCannotUpdate, !bAllow);
	}

	/// @brief Terminates the instance
	/// @note NOP if invoked during updating or signal propagation
	void Widget::Destroy (void)
	{
		if (mState->mMode != eNormal) return;

		// Ensure that the widget is unloaded.
		Unload();

		// Destroy the widget.
		delete this;
	}

	/// @brief Docks a widget
	/// @param widget Handle to the widget
	/// @note NOP if invoked from an update or signal callback
	void Widget::Dock (Widget * widget)
	{
		if (0 == widget) return;
		if (this == widget) return;
		if (mState != widget->mState) return;
		if (eSignalTesting == mState->mMode) return;
		if (eUpdating == mState->mMode) return;

		// Unload the widget, put it in the dock, and bind it and the parent to one another.
		widget->Unload();

		widget->mLookup = mDock.insert(mDock.end(), widget);
		widget->mParent = this;
	}

	/// @brief Puts the widget at the head of its parent's dock
	/// @note NOP if invoked from an update or signal callback
	void Widget::PromoteToDockHead (void)
	{
		if (!IsDocked()) return;
		if (eSignalTesting == mState->mMode) return;
		if (eUpdating == mState->mMode) return;

		mParent->mDock.splice(mParent->mDock.begin(), mParent->mDock, mLookup);
	}

	/// @brief Puts the widget at the head of its frame
	/// @note NOP if invoked from an update or signal callback
	void Widget::PromoteToFrameHead (void)
	{
		if (!IsFramed()) return;
		if (eSignalTesting == mState->mMode) return;
		if (eUpdating == mState->mMode) return;

		mState->mFrame.splice(mState->mFrame.begin(), mState->mFrame, mLookup);
	}

	/// @brief Removes the widget from its parent or the frame
	/// @note NOP if invoked from an update or signal callback
	void Widget::Unload (void)
	{
		if (eSignalTesting == mState->mMode) return;
		if (eUpdating == mState->mMode) return;

		if (IsLoaded())
		{
			// Clear the choice if it is this widget.
			if (mState->GetChoice() == this) mState->ClearChoice();

			// Remove if active in the state.
			if (mState->GetEntered() == this) mState->mEntered = 0;
			if (mState->GetGrabbed() == this) mState->mGrabbed = 0;

			// If the widget is docked, unbind it from the parent.
			if (IsDocked())
			{
				mParent->mDock.erase(mLookup);

				mParent = 0;
			}

			// Otherwise, remove the widget from the frame.
			else
			{
				mState->mFrame.erase(mLookup);

				mStatus.reset(eFramed);
			}
		}
	}

	/// @brief Adds a custom part
	/// @return 0 on failure; part on success
	Part * Widget::CreatePart (void)
	{
		Part * part = 0;

		try {
			part = new Part(this);

			mParts.insert(part);
		} catch (std::bad_alloc &) {}

		return part;
	}

	/// @brief Gets the widget's owner state
	/// @return State to which widget belongs
	State * Widget::GetState (void)
	{
		return mState;
	}

	/// @brief Gets the head of the dock
	/// @return 0 on failure; head on success
	Widget * Widget::GetDockHead (void)
	{
		if (mDock.empty()) return 0;

		return mDock.front();
	}

	/// @brief Gets the next widget in its parent's dock
	/// @return 0 on failure; next widget on success
	Widget * Widget::GetNextDockLink (void)
	{
		if (!IsDocked()) return 0;

		Iter_L iter = mLookup;

		if (++iter == mParent->mDock.end()) return 0;

		return *iter;
	}

	/// @brief Gets the next widget in the frame
	/// @return 0 on failure; next widget on success
	Widget * Widget::GetNextFrameLink (void)
	{
		if (!IsFramed()) return 0;

		Iter_L iter = mLookup;

		if (++iter == mState->mFrame.end()) return 0;

		return *iter;
	}

	/// @brief Gets the parent in which the widget is docked
	/// @return 0 on failure; parent on success
	Widget * Widget::GetParent (void)
	{
		return mParent;
	}

	/// @brief Indicates whether the widget is docked
	/// @return If true, the widget is docked
	bool Widget::IsDocked (void)
	{
		return mParent != 0;
	}

	/// @brief Indicates whether the pane is set in a frame
	/// @return If true, the pane is framed
	bool Widget::IsFramed (void)
	{
		return mStatus.test(eFramed);
	}

	/// @brief Indicates whether the widget is docked or framed
	/// @return If true, the widget is loaded
	bool Widget::IsLoaded (void)
	{
		return IsDocked() || IsFramed();
	}

	/// @brief Indicates whether the widget can be signaled
	/// @return If true, the widget can be signaled
	bool Widget::IsSignalAllowed (void)
	{
		// A widget can be signaled if it is loaded and all its parents allow it.
		if (!IsLoaded()) return false;
		if (mStatus.test(eCannotSignal)) return false;

		Widget * widget = this;

		while (widget->IsDocked())
		{
			widget = widget->mParent;

			if (widget->mStatus.test(eCannotDockSignal)) return false;
		}

		return true;
	}

	/// @brief Indicates whether the widget can be updated
	/// @return If true, the widget can be updated
	bool Widget::IsUpdateAllowed (void)
	{
		// A widget can be updated if it is loaded and all its parents allow it.
		if (!IsLoaded()) return false;
		if (mStatus.test(eCannotUpdate)) return false;

		Widget * widget = this;

		while (widget->IsDocked())
		{
			widget = widget->mParent;

			if (widget->mStatus.test(eCannotDockUpdate)) return false;
		}

		return true;
	}
}