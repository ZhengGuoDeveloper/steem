
#include <steemit/chain/util/reward.hpp>
#include <steemit/chain/util/uint256.hpp>

namespace steemit { namespace chain { namespace util {

uint64_t get_rshare_reward( const comment_reward_context& ctx )
{
   try
   {
   FC_ASSERT( ctx.rshares > 0 );
   FC_ASSERT( ctx.total_reward_shares2 > 0 );

   u256 rs(ctx.rshares.value);
   u256 rf(ctx.total_reward_fund_steem.amount.value);
   u256 total_rshares2 = to256( ctx.total_reward_shares2 );

   //idump( (ctx) );

   u256 rs2 = to256( calculate_claims( ctx.rshares.value ) );
   rs2 = ( rs2 * ctx.reward_weight ) / STEEMIT_100_PERCENT;

   u256 payout_u256 = ( rf * rs2 ) / total_rshares2;
   FC_ASSERT( payout_u256 <= u256( uint64_t( std::numeric_limits<int64_t>::max() ) ) );
   uint64_t payout = static_cast< uint64_t >( payout_u256 );

   if( is_comment_payout_dust( ctx.current_steem_price, payout ) )
      payout = 0;

   asset max_steem = to_steem( ctx.current_steem_price, ctx.max_sbd );

   payout = std::min( payout, uint64_t( max_steem.amount.value ) );

   return payout;
   } FC_CAPTURE_AND_RETHROW( (ctx) )
}

uint64_t get_rshare_reward( const comment_reward_context& ctx, const reward_fund_object& rf_object )
{
   try
   {
   FC_ASSERT( ctx.rshares > 0 );
   FC_ASSERT( ctx.total_reward_shares2 > 0 );

   u256 rf(ctx.total_reward_fund_steem.amount.value);
   u256 total_claims = to256( ctx.total_reward_shares2 );

   //idump( (ctx) );

   u256 claim = to256( calculate_claims( ctx.rshares.value, rf_object ) );
   claim = ( claim * ctx.reward_weight ) / STEEMIT_100_PERCENT;

   u256 payout_u256 = ( rf * claim ) / total_claims;
   FC_ASSERT( payout_u256 <= u256( uint64_t( std::numeric_limits<int64_t>::max() ) ) );
   uint64_t payout = static_cast< uint64_t >( payout_u256 );

   if( is_comment_payout_dust( ctx.current_steem_price, payout ) )
      payout = 0;

   asset max_steem = to_steem( ctx.current_steem_price, ctx.max_sbd );

   payout = std::min( payout, uint64_t( max_steem.amount.value ) );

   return payout;
   } FC_CAPTURE_AND_RETHROW( (ctx) )
}

uint64_t get_vote_weight( uint128_t vote_rshares, const reward_fund_object& rf )
{
   uint64_t result = 0;
   if( rf.name == STEEMIT_POST_REWARD_FUND_NAME || rf.name == STEEMIT_COMMENT_REWARD_FUND_NAME )
   {
      uint128_t two_alpha = rf.content_constant * 2;
      result = ( ( std::numeric_limits< uint64_t >::max() * vote_rshares ) / ( two_alpha + vote_rshares ) ).to_uint64();
   }
   else
   {
      wlog( "Unknown reward fund type ${rf}", ("rf",rf.name) );
   }

   return result;
}

uint128_t calculate_claims( const uint128_t& rshares )
{
   uint128_t s = get_content_constant_s();
   uint128_t rshares_plus_s = rshares + s;
   return rshares_plus_s * rshares_plus_s - s * s;
}

uint128_t calculate_claims( const uint128_t& rshares, const reward_fund_object& rf )
{
   uint128_t result = 0;
   if( rf.name == STEEMIT_POST_REWARD_FUND_NAME || rf.name == STEEMIT_COMMENT_REWARD_FUND_NAME )
   {
      // k * ( ( r + a )^2 - a^2 ) / ( r + 40a )
      uint128_t rshares_plus_a = rshares + rf.content_constant;
      uint128_t rshares_plus_forty_a = rshares + rf.content_constant * 40;
      result = ( STEEMIT_REWARD_SCALING_CONSTANT_K * ( rshares_plus_a * rshares_plus_a - rf.content_constant * rf.content_constant ) ) / rshares_plus_forty_a;
   }
   else
   {
      wlog( "Unknown reward fund type ${rf}", ("rf",rf.name) );
   }

   return result;
}

} } }
