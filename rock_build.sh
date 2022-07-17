package="qfsm"
artifactory_login="dkudrin"


artifactory_path="https://unixrepo.megafon.ru/artifactory/mfactory-generic-local/product-inventory-cache/$package"
read -sp "Enter host password for user '$artifactory_login':" artifactory_password
echo
ver="$(git describe --tags --long)"
v=$(echo $ver | sed 's/-[0-9]-g.*$//')
rel=$(echo $ver | sed 's/^.*-\([0-9]\)-g.*$/\1/')
let "new_rel=$rel+1"
rock_ver="$v-$new_rel"
echo "$package version: $v ($ver). rockspec version: $rock_ver"

rockspec="$package-$rock_ver.rockspec"
rock="$package-$rock_ver.linux-x86_64.rock"

rm -f $package-*.rockspec
rm -f $package-*.rock
rm -f manifest
cp $package-scm-1.rockspec.sample $rockspec
sed -i "s/scm-1/$rock_ver/g"  $rockspec
echo "rockspec file created: $rockspec"
git add . || exit 1
git commit -m "Increase rockspec version to $rock_ver" || exit 1

man_add="\n\t[\"$rock_ver\"] = { { arch = \"linux-x86_64\" } },"

echo "Uploading $rock to artifactory"
tarantoolctl rocks make || exit 1
tarantoolctl rocks pack --pack-binary-rock $package || exit 1
curl -ku"$artifactory_login:$artifactory_password" -T "$rock" "$artifactory_path/$rock" || exit 1

echo "Adding $rock_ver to manifest"
# Manifest editing
curl -k -O "$artifactory_path/manifest"  || exit 1
sed -i "/\[\"$rock_ver\"\]/d" manifest
sed -i "s/qfsm = {/qfsm = {$(echo $man_add)/" manifest

curl -ku"$artifactory_login:$artifactory_password" -T manifest "$artifactory_path/manifest" || exit 1
rm manifest
rm $rock
