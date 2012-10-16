#include "UserInterface.h"

namespace UI
{
	/// @brief Constructs a Widget object
	/// @param state State used to build widget
	Widget::Widget (State * state) : Signalee(), mState(state), mParent(0)
	{
	}

	/// @brief Destructs a Widget object
	Widget::~Widget (void)
	{
		while (!mParts.empty()) (*mParts.begin())->Remove();
		while (!mDock.empty()) mDock.front()->Unload();

		mState->mWidgets.erase(this);
	}

	/// @brief Performs choose logic
	/// @param bPressed If true, there is a press
	void Widget::Choose (bool bPressed)
	{
		IssueEvent(ePreChoose);

		Enter();

		if (bPressed) Grab();

		IssueEvent(ePostChoose);
	}

	/// @brief Issues an event to the widget
	/// @param event Type of event to issue
	void Widget::IssueEvent (Event event)
	{
		mState->mEventFunc(this, event);
	}

	/// @brief Runs a signal test on the widget and through its dock
	void Widget::SignalTest (void)
	{
		// Iterate through the dock, recursing on each widget.
		for (Iter_L wIter = mDock.begin(); wIter != mDock.end() && !mStatus.test(eCannotDockSignal); ++wIter)
		{
			(*wIter)->SignalTest();
		}

		// Perform the signal test. Testing is complete if a signal is obtained or if the
		// user aborted testing.
		if (!mStatus.test(eCannotSignal))
		{
			mState->mSignalFunc(this);

			if (mState->mSignaled != 0) throw true;
			if (eNormal == mState->mMode) throw false;
		}
	}

	/// @brief Performs an update on the widget and through its dock
	void Widget::Update (void)
	{
		if (!mStatus.test(eCannotUpdate)) mState->mUpdateFunc(this);

		// Iterate backward through the dock, recursing on each widget.
		for (RIter_L wIter = mDock.rbegin(); wIter != mDock.rend() && !mStatus.test(eCannotDockUpdate); ++wIter)
		{
			(*wIter)->Update();
		}
	}

	/// @brief Performs upkeep logic
	/// @param bPressed If true, there is a press
	void Widget::Upkeep (bool bPressed)
	{
		IssueEvent(ePreUpkeep);

		// Perform leave logic.
		Leave();

		// If the widget is signaled, perform enter logic; on a press, perform grab logic.
		if (IsSignaled())
		{
			Enter();

			if (bPressed) Grab();
		}

		// If there is no press, perform drop logic.
		if (!bPressed) Drop();

		// If the widget remains chosen, post-process it; otherwise, abandon it.
		if (!IsSignaled() && !IsChosen())
		{
			mState->ClearChoice();

			IssueEvent(eAbandon);
		}

		else IssueEvent(ePostUpkeep);
	}

	/// @brief Performs drop logic
	void Widget::Drop (void)
	{
		if (0 == mState->mGrabbed) return;

		IssueEvent(IsGrabbed() ? eDrop : eDropPart);

		mState->mGrabbed = 0;
	}

	/// @brief Performs enter logic
	void Widget::Enter (void)
	{
		if (mState->mEntered == mState->mSignaled) return;

		mState->mEntered = mState->mSignaled;

		IssueEvent(IsEntered() ? eEnter : eEnterPart);
	}

	/// @brief Performs grab logic
	void Widget::Grab (void)
	{
		if (mState->mGrabbed != 0) return;

		mState->mGrabbed = mState->mSignaled;

		IssueEvent(IsGrabbed() ? eGrab : eGrabPart);
	}

	/// @brief Performs leave logic
	void Widget::Leave (void)
	{
		if (0 == mState->mEntered || mState->mEntered == mState->mSignaled) return;

		IssueEvent(IsEntered() ? eLeave : eLeavePart);

		mState->mEntered = 0;
	}

	/// @brief Gets the widget's owner
	/// @return Owner widget
	Widget * Widget::GetOwner (void)
	{
		return this;
	}

	/// @brief Indicates whether the widget is chosen
	/// @return If true, the widget is chosen
	bool Widget::IsChosen (void)
	{
		return mState->mGrabbed != 0 && mState->mGrabbed->GetOwner() == this;
	}
}