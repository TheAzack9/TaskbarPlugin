#pragma once
class Utility
{
public:
	static Utility& GetUtility();

	// Non-copyable
	Utility(Utility const&) = delete;
	void operator=(Utility const&) = delete;

	void SetRm(void* rm);

private:
	Utility();
	~Utility();

	void* m_Rm;
};

