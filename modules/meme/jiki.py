import traceback

from bs4 import BeautifulSoup

from config import CFG
from core.builtins import Url
from core.logger import Logger
from core.utils.http import get_url
from core.utils.i18n import Locale

web_render = CFG.get_url('web_render')
web_render_local = CFG.get_url('web_render_local')

async def jiki(term: str, locale: Locale):
    '''查询小鸡百科。

    :param term: 需要查询的term。
    :returns: 查询结果。'''
    try:
        api = 'https://jikipedia.com/search?phrase=' + term
        if web_render:
            use_local = True if web_render_local else False
        else:
            return
        api = (web_render_local if use_local else web_render) + 'source?url=' + api
        html = await get_url(api, 200)
        Logger.debug(html)
        bs = BeautifulSoup(html, 'html.parser')
        result = bs.select_one('[data-index="0"]')
        title_ele = result.select_one(
            'a.title-container.block.title-normal')
        content_ele = result.select_one('.lite-card-content')

        title = title_ele.get_text()
        link = title_ele.get('href')
        content = content_ele.get_text()

        results = bs.select('.lite-card').__len__()
        count = str(result) if results < 15 else '15+'
        return f'[{locale.t("meme.message.jiki")}] {locale.t("meme.message.result", result=count)}{title}\n{content}\n{str(Url(link))}'
    except Exception:
        traceback.print_exc()
        return f'[{locale.t("meme.message.jiki")}] {locale.t("meme.message.error")}'