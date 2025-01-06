import { sha256hash } from "./util.js";

export async function isFingerprintAllowed(subkey, key) {
  // sha256 of an empty hash is e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855
  const subkeyList = {
    [await sha256hash('SOFTWARE\\STRIPPED_FOR_PUBLIC_RELEASE\\STRIPPED_FOR_PUBLIC_RELEASE')]: [await sha256hash('Language'), await sha256hash('STRIPPED_FOR_PUBLIC_RELEASE')],
    [await sha256hash('SOFTWARE\\STRIPPED_FOR_PUBLIC_RELEASE STRIPPED_FOR_PUBLIC_RELEASE')]: await sha256hash('STRIPPED_FOR_PUBLIC_RELEASE'),
    [await sha256hash('SOFTWARE\\STRIPPED_FOR_PUBLIC_RELEASE')]: [await sha256hash('STRIPPED_FOR_PUBLIC_RELEASE')],
    [await sha256hash('SOFTWARE\\STRIPPED_FOR_PUBLIC_RELEASE\\STRIPPED_FOR_PUBLIC_RELEASE\\STRIPPED_FOR_PUBLIC_RELEASE')]: await sha256hash('STRIPPED_FOR_PUBLIC_RELEASE'),
	  [await sha256hash('SOFTWARE\\STRIPPED_FOR_PUBLIC_RELEASE\\STRIPPED_FOR_PUBLIC_RELEASE\\STRIPPED_FOR_PUBLIC_RELEASE')]: await sha256hash('STRIPPED_FOR_PUBLIC_RELEASE'),
    [await sha256hash('SOFTWARE\\STRIPPED_FOR_PUBLIC_RELEASE\\STRIPPED_FOR_PUBLIC_RELEASE\\STRIPPED_FOR_PUBLIC_RELEASE')]: await sha256hash('')
  };

  return Object.hasOwn(subkeyList, subkey) && (subkeyList[subkey] === key || subkeyList[subkey].includes(key));
}