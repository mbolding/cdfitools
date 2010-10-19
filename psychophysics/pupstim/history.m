%-- 1/14/08 11:02 PM --%
clear

load sub05arb.txt
sub05bk.txt
sub05mcd.txt
sub05envtimes.txt
sub05texttimes.txt
sub05ititimes.txt
sub05none.txt
load sub05bk.txt
load sub05mcd.txt
load sub05envtimes.txt
load sub05texttimes.txt
load sub05ititimes.txt
load sub05none.txt
save sub05
ls
plot(t,p)
plot(p)

save sub05
rewards = sort([sub05arb ; sub05bk; sub05mcd]);
save sub05
intersect(rewards,sub05none)
union(rewards,sub05none)

sub05plots
importfile_sub05('sub05samps3.txt')
sub05plots
save sub05
