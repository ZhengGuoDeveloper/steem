#include <steemit/chain/account_object.hpp>

#include <steemit/follow/follow_api.hpp>

namespace steemit { namespace follow {

namespace detail
{

class follow_api_impl
{
   public:
      follow_api_impl( steemit::app::application& _app )
         :app(_app) {}

      vector< follow_object > get_followers( string following, string start_follower, follow_type type, uint16_t limit )const;
      vector< follow_object > get_following( string follower, string start_following, follow_type type, uint16_t limit )const;

      vector< feed_entry > get_feed_entries( string account, uint32_t entry_id, uint16_t limit )const;
      vector< comment_feed_entry > get_feed( string account, uint32_t entry_id, uint16_t limit )const;

      vector< blog_entry > get_blog_entries( string account, uint32_t entry_id, uint16_t limit )const;
      vector< comment_blog_entry > get_blog( string account, uint32_t entry_id, uint16_t limit )const;

      vector< account_reputation > get_account_reputations( string lower_bound_name, uint32_t limit )const;

      steemit::app::application& app;
};

vector< follow_object > follow_api_impl::get_followers( string following, string start_follower, follow_type type, uint16_t limit )const
{
   FC_ASSERT( limit <= 100 );
   vector<follow_object> result;
   const auto& idx = app.chain_database()->get_index_type<follow_index>().indices().get<by_following_follower>();
   const auto& following_obj = app.chain_database()->get_account( following );
   const auto& start_follower_obj = app.chain_database()->get_account( start_follower );
   auto itr = idx.lower_bound( std::make_tuple( following_obj.id, start_follower_obj.id ) );
   while( itr != idx.end() && limit && itr->following == following_obj.id )
   {
      if( itr->what.find( type ) != itr->what.end() )
      {
         result.push_back( *itr );
         --limit;
      }

      ++itr;
   }

   return result;
}

vector< follow_object > follow_api_impl::get_following( string follower, string start_following, follow_type type, uint16_t limit )const
{
   FC_ASSERT( limit <= 100 );
   vector<follow_object> result;
   const auto& idx = app.chain_database()->get_index_type<follow_index>().indices().get<by_follower_following>();
   const auto& follower_obj = app.chain_database()->get_account( follower );
   const auto& start_following_obj = app.chain_database()->get_account( start_following );
   auto itr = idx.lower_bound( std::make_tuple( follower_obj.id, start_following_obj.id ) );
   while( itr != idx.end() && limit && itr->follower == follower_obj.id )
   {
      if( itr->what.find( type ) != itr->what.end() )
      {
         result.push_back( *itr );
         --limit;
      }

      ++itr;
   }

   return result;
}

vector< feed_entry > follow_api_impl::get_feed_entries( string account, uint32_t entry_id, uint16_t limit )const
{
   FC_ASSERT( limit <= 500, "Cannot retrieve more than 500 feed entries at a time." );

   if( entry_id == 0 )
      entry_id = ~0;

   vector< feed_entry > results;
   results.reserve( limit );

   const auto& db = *app.chain_database();
   const auto& acc_id = db.get_account( account ).id;
   const auto& feed_idx = db.get_index_type< feed_index >().indices().get< by_feed >();
   auto itr = feed_idx.lower_bound( boost::make_tuple( acc_id, entry_id ) );

   while( itr != feed_idx.end() && itr->account == acc_id && results.size() < limit )
   {
      const auto& comment = itr->comment( db );
      feed_entry entry;
      entry.author = comment.author;
      entry.permlink = comment.permlink;
      entry.entry_id = itr->account_feed_id;
      if( itr->first_reblogged_by != account_id_type() )
      {
         entry.reblog_by = itr->first_reblogged_by(db).name;
         entry.reblog_on = itr->first_reblogged_on;
      }
      results.push_back( entry );

      ++itr;
   }

   return results;
}

vector< comment_feed_entry > follow_api_impl::get_feed( string account, uint32_t entry_id, uint16_t limit )const
{
   FC_ASSERT( limit <= 500, "Cannot retrieve more than 500 feed entries at a time." );

   if( entry_id == 0 )
      entry_id = ~0;

   vector< comment_feed_entry > results;
   results.reserve( limit );

   const auto& db = *app.chain_database();
   const auto& acc_id = app.chain_database()->get_account( account ).id;
   const auto& feed_idx = app.chain_database()->get_index_type< feed_index >().indices().get< by_feed >();
   auto itr = feed_idx.lower_bound( boost::make_tuple( acc_id, entry_id ) );

   while( itr != feed_idx.end() && itr->account == acc_id && results.size() < limit )
   {
      const auto& comment = itr->comment( *app.chain_database() );
      comment_feed_entry entry;
      entry.comment = comment;
      entry.entry_id = itr->account_feed_id;
      if( itr->first_reblogged_by != account_id_type() )
      {
         entry.reblog_by = itr->first_reblogged_by(db).name;
         entry.reblog_on = itr->first_reblogged_on;
      }
      results.push_back( entry );

      ++itr;
   }

   return results;
}

vector< blog_entry > follow_api_impl::get_blog_entries( string account, uint32_t entry_id, uint16_t limit )const
{
   FC_ASSERT( limit <= 500, "Cannot retrieve more than 500 blog entries at a time." );

   if( entry_id == 0 )
      entry_id = ~0;

   vector< blog_entry > results;
   results.reserve( limit );

   const auto& db = *app.chain_database();
   const auto& acc_id = db.get_account( account ).id;
   const auto& blog_idx = db.get_index_type< blog_index >().indices().get< by_blog >();
   auto itr = blog_idx.lower_bound( boost::make_tuple( acc_id, entry_id ) );

   while( itr != blog_idx.end() && itr->account == acc_id && results.size() < limit )
   {
      const auto& comment = itr->comment( db );
      blog_entry entry;
      entry.author = comment.author;
      entry.permlink = comment.permlink;
      entry.blog = account;
      entry.reblog_on = itr->reblogged_on;
      entry.entry_id = itr->blog_feed_id;

      results.push_back( entry );

      ++itr;
   }

   return results;
}

vector< comment_blog_entry > follow_api_impl::get_blog( string account, uint32_t entry_id, uint16_t limit )const
{
   FC_ASSERT( limit <= 500, "Cannot retrieve more than 500 blog entries at a time." );

   if( entry_id == 0 )
      entry_id = ~0;

   vector< comment_blog_entry > results;
   results.reserve( limit );

   const auto& db = *app.chain_database();
   const auto& acc_id = db.get_account( account ).id;
   const auto& blog_idx = db.get_index_type< blog_index >().indices().get< by_blog >();
   auto itr = blog_idx.lower_bound( boost::make_tuple( acc_id, entry_id ) );

   while( itr != blog_idx.end() && itr->account == acc_id && results.size() < limit )
   {
      const auto& comment = itr->comment( db );
      comment_blog_entry entry;
      entry.comment = comment;
      entry.blog = account;
      entry.reblog_on = itr->reblogged_on;
      entry.entry_id = itr->blog_feed_id;

      results.push_back( entry );

      ++itr;
   }

   return results;
}

vector< account_reputation > follow_api_impl::get_account_reputations( string lower_bound_name, uint32_t limit )const
{
   FC_ASSERT( limit <= 1000, "Cannot retrieve more than 1000 account reputations at a time." );

   const auto& acc_idx = app.chain_database()->get_index_type< account_index >().indices().get< by_name >();
   const auto& rep_idx = app.chain_database()->get_index_type< reputation_index >().indices().get< by_account >();

   auto acc_itr = acc_idx.lower_bound( lower_bound_name );

   vector< account_reputation > results;
   results.reserve( limit );

   while( acc_itr != acc_idx.end() && results.size() < limit )
   {
      auto itr = rep_idx.find( acc_itr->id );
      account_reputation rep;

      rep.account = acc_itr->name;
      rep.reputation = itr != rep_idx.end() ? itr->reputation : 0;

      results.push_back( rep );

      ++acc_itr;
   }

   return results;
}

} // detail

follow_api::follow_api( const steemit::app::api_context& ctx )
{
   my = std::make_shared< detail::follow_api_impl >( ctx.app );
}

void follow_api::on_api_startup() {}

vector<follow_object> follow_api::get_followers( string following, string start_follower, follow_type type, uint16_t limit )const
{
   return my->get_followers( following, start_follower, type, limit );
}

vector<follow_object> follow_api::get_following( string follower, string start_following, follow_type type, uint16_t limit )const
{
   return my->get_following( follower, start_following, type, limit );
}

vector< feed_entry > follow_api::get_feed_entries( string account, uint32_t entry_id, uint16_t limit )const
{
   return my->get_feed_entries( account, entry_id, limit );
}

vector< comment_feed_entry > follow_api::get_feed( string account, uint32_t entry_id, uint16_t limit )const
{
   return my->get_feed( account, entry_id, limit );
}

vector< blog_entry > follow_api::get_blog_entries( string account, uint32_t entry_id, uint16_t limit )const
{
   return my->get_blog_entries( account, entry_id, limit );
}

vector< comment_blog_entry > follow_api::get_blog( string account, uint32_t entry_id, uint16_t limit )const
{
   return my->get_blog( account, entry_id, limit );
}

vector< account_reputation > follow_api::get_account_reputations( string lower_bound_name, uint32_t limit )const
{
   return my->get_account_reputations( lower_bound_name, limit );
}

} } // steemit::follow
