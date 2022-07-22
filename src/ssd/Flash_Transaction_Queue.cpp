#include "Flash_Transaction_Queue.h"

namespace SSD_Components
{
	Flash_Transaction_Queue::Flash_Transaction_Queue() {}

	Flash_Transaction_Queue::Flash_Transaction_Queue(std::string id) : id(id)
	{
	}

	void Flash_Transaction_Queue::SetID(std::string id)
	{
		this->id = id;
	}

	void Flash_Transaction_Queue::push_back(NVM_Transaction_Flash* const& transaction)
	{
		RequestQueueProbe.EnqueueRequest(transaction);
		return list<NVM_Transaction_Flash*>::push_back(transaction);
	}

	void Flash_Transaction_Queue::push_front(NVM_Transaction_Flash* const& transaction)
	{
		RequestQueueProbe.EnqueueRequest(transaction);
		return list<NVM_Transaction_Flash*>::push_front(transaction);
	}

	std::list<NVM_Transaction_Flash*>::iterator Flash_Transaction_Queue::insert(list<NVM_Transaction_Flash*>::iterator position, NVM_Transaction_Flash* const& transaction)
	{
		RequestQueueProbe.EnqueueRequest(transaction);
		return list<NVM_Transaction_Flash*>::insert(position, transaction);
	}

	void Flash_Transaction_Queue::remove(NVM_Transaction_Flash* const& transaction)
	{
		RequestQueueProbe.DequeueRequest(transaction);
		return list<NVM_Transaction_Flash*>::remove(transaction);
	}

	void Flash_Transaction_Queue::remove(std::list<NVM_Transaction_Flash*>::iterator const& itr_pos)
	{
		RequestQueueProbe.DequeueRequest(*itr_pos);
		list<NVM_Transaction_Flash*>::erase(itr_pos);
	}

	void Flash_Transaction_Queue::pop_front()
	{
		RequestQueueProbe.DequeueRequest(this->front());
		list<NVM_Transaction_Flash*>::pop_front();
	}

	void Flash_Transaction_Queue::ReportResultsInXML(std::string name_prefix, Utils::XmlWriter& xmlwriter)
	{
		std::string tmp = name_prefix;
		xmlwriter.WriteStartElementTag(tmp);

		std::string attr = "Name";
		std::string val = id;
		xmlwriter.WriteAttributeStringInline(attr, val);

		attr = "No_Of_Transactions_Enqueued";
		val = std::to_string(RequestQueueProbe.NRequests());
		xmlwriter.WriteAttributeStringInline(attr, val);

		attr = "No_Of_Transactions_Dequeued";
		val = std::to_string(RequestQueueProbe.NDepartures());
		xmlwriter.WriteAttributeStringInline(attr, val);

		attr = "Avg_Queue_Length";
		val = std::to_string(RequestQueueProbe.AvgQueueLength());
		xmlwriter.WriteAttributeStringInline(attr, val);

		attr = "Max_Queue_Length";
		val = std::to_string(RequestQueueProbe.MaxQueueLength());
		xmlwriter.WriteAttributeStringInline(attr, val);

		attr = "STDev_Queue_Length";
		val = std::to_string(RequestQueueProbe.STDevQueueLength());
		xmlwriter.WriteAttributeStringInline(attr, val);

		attr = "Avg_Transaction_Waiting_Time";
		val = std::to_string(RequestQueueProbe.AvgWaitingTime());
		xmlwriter.WriteAttributeStringInline(attr, val);

		attr = "Max_Transaction_Waiting_Time";
		val = std::to_string(RequestQueueProbe.MaxWaitingTime());
		xmlwriter.WriteAttributeStringInline(attr, val);

		xmlwriter.WriteEndElementTag();
	}
}
