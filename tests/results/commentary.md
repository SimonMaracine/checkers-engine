# Commentary

## 2.0 black vs 2.0 white | match--Nov-28--13-22-46--2024.json

Engine 2.0 against itself is not very interesting, but it proves that the match execution is fine.

## 2.0 black vs 3.0 white | match--Nov-29--12-17-36--2024.json

Engine 3.0 won the match right on the edge. Iteratve deepening + transposition tables seem to be superior to plain
fixed depth frameworks.

## 3.0 black vs 4.0 white | match--Nov-30--22-55-20--2024.json

Engine 4.0 won the match and crushed the previous version engine. The PV in each iterative deepening iteration
really seems to help.

## 4.0 black vs 5.0 white | match--Dec-2--13-34-17--2024.json

Engine 5.0 won the match and crushed the previous version engine. Not a surprise, since the newer version has
a much improved evaluation function.

## 5.0 black vs 6.0 white | match--Dec-2--21-29-33--2024.json

Engine 5.0, surprisingly, won the match. Engine 6.0 contains no other but quality changes. I expected to be at
least as good as engine 5.0, if not better.

## 5.0 black vs 6.1 white | match--Dec-3--00-58-43--2024.json

Engine 5.0 won again. I thought I fixed the issue, which turned out not to be at all an issue (right now). Instead,
engine 6.1 played slightly worse.

## 6.1 black vs 5.0 white | match--Dec-3--16-26-03--2024.json

I swapped the colors and ran the same match again hoping to find a bug in the Comparator program. This time,
not surprisingly, engine 5.0 won the match. But engine 6.1 played a bit better.

I think that the newer engine doesn't actually have a bug, instead it cannot search deeply enough in the middle game.
The change made it so that the moves are generated in every single node, whereas before they were only generated,
if depth > 0 or game is over. The newer version thus does a lot more work in the alpha-beta search, while saving
some time not evaluating the game over nodes in the end game. The result is that, on average, it searches as deep
as version 5.0, but it performs slightly worse.

## 6.1 black vs 7.0 white | match--Dec-3--23-59-43--2024.json

Engine 7.0 won against 6.1, having only some tweaked parameters and a few changes to its evaluation function.

## 7.0 black vs 8.0 white | match--Dec-4--15-30-30--2024.json

Engine 8.0 lost against the previous version. It seems that the little changes that I made were not beneficial.

## 7.0 black vs 8.1 white | match--Dec-4--18-13-45--2024.json

Engine 8.1 lost against engine 7.0. The changes made insignificant improvements.

## 7.0 black vs 8.2 white | match--Dec-4--20-47-18--2024.json

The match turned out to be a tie. I'm keeping these changes, since they make much more sense than engine 7.0's state,
so I will build upon this newest version.

## 8.2 black vs 9.0 white | match--Dec-5--15-19-07--2024.json

Engine 9.0 won the match by far, which pleasantly surprised me. I didn't expect the quiescent search to be that
effective.

## 9.0 black vs 10.0 white | match--Dec-6--16-49-38--2024.json

Engine 10.0 won, but not by much. It was very close to be a tie. The savings in space didn't have a large effect
this time.

## 10.0 black vs 11.0 white | match--Dec-9--00-30-46--2024.json

Engine 11.0 won the match. The many little optimizations paid off. Engine 11.0's average search depth is indeed a
little higher.

## 11.1 black vs 12.0 white | match--Dec-11--14-06-34--2024.json

The match results were dead equal. Engine 12.0, despite having a much better TT implementation, searched
shallower on average, which is very contradictory.

## 11.1 black vs 12.1 white | match--Dec-11--17-43-22--2024.json

The match ended up still being equal. A smaller TT size didn't help.

## 11.1 black vs 12.2 white | match--Dec-12--00-43-24--2024.json

Engine 12.2 won the match. It seems that a larger TT is actually better, but not by much. It is still a mistery
to me that fact that engine 12.2 still searched on average a bit shallower.

## 12.2 black vs 13.0 white | match--Dec-12--19-12-14--2024.json

Engine 13.0 won the match by a decent margin. Taking into account the hash move from the TT in move reordering
is a good idea.

## 13.0 black vs 14.0 white | match--Dec-13--17-35-24--2024.json

Engine 14.0 won the match, but not by much at all. I expected not clearing the TT before every move to improve
things more than this.

## 14.0 black vs 15.0 white | match--Dec-14--10-58-03--2024.json

Engine 15.0 won, but the match was very close to being even.

## 15.0 black vs 16.0 white | match--Dec-14--14-24-16--2024.json

Engine 16.0 won the match. I expected the aspiration windows to have a bigger benefit though.

## 16.0 black vs 17.0 white | match--Dec-17--15-23-20--2024.json

Engine 17.0 won, but the difference in wins is insignificant.
